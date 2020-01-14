/*TODO:
[ ] Maybe save the line number on the stack with each token so that errors
    can be more informative
[ ] Add support for ordered and unordered lists
[ ] Make some of the code for similar tags (e.g. <em> and <strong>) reusable
*/
#include <iostream>
#include <fstream>
#include <utility> //for std::pair
#include <stack>
#include <map>

enum class State : char {
    Start, Bold, Italic, Heading, Bad, EoF, Text, Paragraph, Newline,
    UnorderedList, OrderedList
};

// Inline elements
constexpr char StrongOpen[] = "<strong>";
constexpr char StrongClose[] = "</strong>";
constexpr char ItalicOpen[] = "<em>";
constexpr char ItalicClose[] = "</em>";
// Block elements
constexpr char H1Open[] = "<h1>";
constexpr char H1Close[] = "</h1>\n";
constexpr char ParagraphOpen[] = "<p>\n";
constexpr char ParagraphClose[] = "\n</p>\n";
constexpr char LiOpen[] = "<li>";
constexpr char LiClose[] = "</li>\n";
constexpr char UlOpen[] = "<ul>\n";
constexpr char UlClose[] = "</ul>\n";
constexpr char OlOpen[] = "<ol>\n";
constexpr char OlClose[] = "</ol>\n";
// Boilerplate HTML around the body
constexpr char HTMLBeforeBody[] = "<html>\n<body>\n";
constexpr char HTMLAfterBody[] = "</body>\n</html>\n";

class Scanner {
private:
    std::ifstream m_file;
public:
    Scanner(const std::string path);
    bool hasNext() const { return !m_file.fail(); }
    std::pair<State,std::string> nextToken();
};

Scanner::Scanner(const std::string path) : m_file(path)
{
    if(path.size() < 3 || path.substr(path.size()-3) != ".md") {
	std::cerr << "Error: must give a .md file\n";
    } else if(!m_file) {
	std::cerr << "Error: could not find file: " << path << '\n';
	exit(1);
    }
}

std::pair<State,std::string> Scanner::nextToken()
{
    bool done = false;
    char currChar = m_file.get();
    State currState = State::Start;
    std::string tokenText;
    while(!done) {
	switch(currState) {
	case State::Start: {
	    if(currChar == '#') {
		currState = State::Heading;
	    } else if(currChar == '-') {
		currState = State::UnorderedList;
	    } else if(std::isdigit(currChar) && m_file.peek() == '.') {
		m_file.ignore();
		currState = State::OrderedList;
	    } else if(currChar == '\n' && m_file.peek() == '\n') {
		m_file.ignore();
		currState = State::Newline;
		done = true;
	    } else if(currChar == '*' && m_file.peek() != '*') {
		currState = State::Italic;
		done = true;
	    } else if(currChar == '*') {
		m_file.ignore();
		currState = State::Bold;
		done = true;
	    } else if(currChar == EOF) {
		currState = State::EoF;
		done = true;
	    } else {
		tokenText += currChar;
		currState = State::Text;
	    }
	    break;
	}
	case State::Heading: {
	    if(currChar == '\n' && m_file.peek() == '\n') {
		m_file.putback(currChar);
		done = true;
	    } else if(currChar == EOF) {
		currState = State::Bad;
		std::cerr << "Error: premature end to Heading element\n";
		done = true;
	    } else {
		tokenText += currChar;
	    }
	    break;
	}
	case State::OrderedList:
	case State::UnorderedList: {
	    if(currChar == '\n' && m_file.peek() == '\n') {
		m_file.putback(currChar);
		done = true;
	    } else if(currChar == '\n') {
		done = true;
	    } else if(currChar == EOF) {
		currState = State::Bad;
		std::cerr << "Error: premature end to list tag\n";
		done = true;
	    } else {
		tokenText += currChar;
	    }
	    break;
	}
	case State::Text: {
	    if(currChar == '\n' && m_file.peek() == '\n') {
		m_file.putback(currChar);
		done = true;
	    } else if(currChar == '\n') {
		tokenText += currChar;
		done = true;
	    } else if(currChar == EOF) {
		done = true;
	    } else if(currChar == '*') {
		m_file.putback(currChar);
	        done = true;
	    } else {
		tokenText += currChar;
	    }
	    break;
	}
	default:
	    currState = State::Bad;
	    done = true;
        }
	
	if(!done) {
	    currChar = m_file.get();
	}
    }
    return {currState, tokenText};
}

int main(int argc, char **argv)
{
    if(argc < 2) {
	std::cout << "Usage: ./markdown <markdown source file>\n";
	exit(1);
    }
    Scanner scanner(argv[1]);
    std::stack<State> stack;
    std::cout << HTMLBeforeBody;
    while(scanner.hasNext()) {
	const auto[state, tokenText] = scanner.nextToken();
	if(state == State::Bold || state == State::Italic) {
	    // <strong> and <em>
	    if(stack.empty() || stack.top() != state) {
		stack.push(state);
	        if(state == State::Bold) {
		    std::cout << StrongOpen;
		} else {
		    std::cout << ItalicOpen;
		}
	    } else if(!stack.empty()){
		stack.pop();
		if(state == State::Bold) {
		    std::cout << StrongClose;
		} else {
		    std::cout << ItalicClose;
		}
	    }
	} else if(state == State::Heading) {
	    // <h1>
	    std::cout << H1Open << tokenText << H1Close;
	} else if(state == State::UnorderedList) {
	    // <ul> and <li>
	    if(stack.empty() || stack.top() != State::UnorderedList) {
		stack.push(state);
		std::cout << UlOpen << LiOpen << tokenText << LiClose;
	    } else {
		std::cout << LiOpen << tokenText << LiClose;
	    }
	} else if(state == State::OrderedList) {
	    // <ol> and <li>
	    if(stack.empty() || stack.top() != State::OrderedList) {
		stack.push(state);
		std::cout << OlOpen << LiOpen << tokenText << LiClose;
	    } else {
		std::cout << LiOpen << tokenText << LiClose;
	    }
	} else if(state == State::Newline && !stack.empty()) {
	    // </ul>, </ol>, and </p>
	    if(stack.top() == State::Paragraph) {
		std::cout << ParagraphClose;
	    } else if(stack.top() == State::UnorderedList) {
		std::cout << UlClose;
	    } else if(stack.top() == State::OrderedList) {
		std::cout << OlClose;
	    }
	    stack.pop();
	} else if(state == State::Text) {
	    // <p>, any text between tags
	    if(stack.empty()) {
		stack.push(State::Paragraph);
		std::cout << ParagraphOpen;
	    }
	    std::cout << tokenText;
	} else if(state == State::Bad) {
	    std::cerr << "Error: Bad token: " << tokenText << "\n";
	}
    }

    // Cleanup/close tags still open at end of file
    State curr;
    while(!stack.empty()) {
        curr = stack.top();
	stack.pop();
	if(curr == State::Paragraph) {
	    // </p>
	    std::cout << ParagraphClose;
	} else if(curr == State::UnorderedList) {
	    // </ul>
	    std::cout << UlClose;
	} else if(curr == State::OrderedList) {
	    // </ol>
	    std::cout << OlClose;
	}
    }
    std::cout << HTMLAfterBody;
    
    return 0;
}
