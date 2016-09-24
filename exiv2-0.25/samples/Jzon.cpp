/*
Copyright (c) 2013 Johannes Häggqvist

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/

#ifdef  _WINDLL
#define JzonAPI __declspec(dllexport)
#endif

#include "Jzon.h"

#include <sstream>
#include <fstream>
#include <stack>
#include <algorithm>

namespace Jzon
{
	class FormatInterpreter
	{
	public:
		FormatInterpreter()
		{
			SetFormat(NoFormat);
		}
		FormatInterpreter(const Format &format)
		{
			SetFormat(format);
		}

		void SetFormat(const Format &format)
		{
			this->format = format;
			indentationChar = (format.useTabs ? '\t' : ' ');
			spacing = (format.spacing ? " " : "");
			newline = (format.newline ? "\n" : spacing);
		}

		std::string GetIndentation(unsigned int level) const
		{
			if (!format.newline)
			{
				return "";
			}
			else
			{
				return std::string(format.indentSize * level, indentationChar);
			}
		}

		inline const std::string &GetNewline() const
		{
			return newline;
		}
		inline const std::string &GetSpacing() const
		{
			return spacing;
		}

	private:
		Format format;
		char indentationChar;
		std::string newline;
		std::string spacing;
	};

	inline bool IsWhitespace(char c)
	{
		return (c == '\n' || c == ' ' || c == '\t' || c == '\r' || c == '\f');
	}
	inline bool IsNumber(char c)
	{
		return ((c >= '0' && c <= '9') || c == '.' || c == '-');
	}

	Node::Node()
	{
	}
	Node::~Node()
	{
	}

	Object &Node::AsObject()
	{
		if (IsObject())
			return static_cast<Object&>(*this);
		else
			throw TypeException();
	}
	const Object &Node::AsObject() const
	{
		if (IsObject())
			return static_cast<const Object&>(*this);
		else
			throw TypeException();
	}
	Array &Node::AsArray()
	{
		if (IsArray())
			return static_cast<Array&>(*this);
		else
			throw TypeException();
	}
	const Array &Node::AsArray() const
	{
		if (IsArray())
			return static_cast<const Array&>(*this);
		else
			throw TypeException();
	}
	Value &Node::AsValue()
	{
		if (IsValue())
			return static_cast<Value&>(*this);
		else
			throw TypeException();
	}
	const Value &Node::AsValue() const
	{
		if (IsValue())
			return static_cast<const Value&>(*this);
		else
			throw TypeException();
	}

	Node::Type Node::DetermineType(const std::string &json)
	{
		std::string::const_iterator jsonIt = json.begin();

		while (jsonIt != json.end() && IsWhitespace(*jsonIt))
			++jsonIt;

		if (jsonIt == json.end())
			return T_VALUE;

		switch (*jsonIt)
		{
		case '{' : return T_OBJECT;
		case '[' : return T_ARRAY;
		default  : return T_VALUE;
		}
	}


	Value::Value() : Node()
	{
		SetNull();
	}
	Value::Value(const Value &rhs) : Node()
	{
		Set(rhs);
	}
	Value::Value(const Node &rhs) : Node()
	{
		const Value &value = rhs.AsValue();
		Set(value);
	}
	Value::Value(ValueType type, const std::string &value)
	{
		Set(type, value);
	}
	Value::Value(const std::string &value)
	{
		Set(value);
	}
	Value::Value(const char *value)
	{
		Set(value);
	}
	Value::Value(const int value)
	{
		Set(value);
	}
	Value::Value(const float value)
	{
		Set(value);
	}
	Value::Value(const double value)
	{
		Set(value);
	}
	Value::Value(const bool value)
	{
		Set(value);
	}
	Value::~Value()
	{
	}

	Node::Type Value::GetType() const
	{
		return T_VALUE;
	}
	Value::ValueType Value::GetValueType() const
	{
		return type;
	}

	std::string Value::ToString() const
	{
		if (IsNull())
		{
			return "null";
		}
		else
		{
			return valueStr;
		}
	}
	int Value::ToInt() const
	{
		if (IsNumber())
		{
			std::stringstream sstr(valueStr);
			int val;
			sstr >> val;
			return val;
		}
		else
		{
			return 0;
		}
	}
	float Value::ToFloat() const
	{
		if (IsNumber())
		{
			std::stringstream sstr(valueStr);
			float val;
			sstr >> val;
			return val;
		}
		else
		{
			return 0.f;
		}
	}
	double Value::ToDouble() const
	{
		if (IsNumber())
		{
			std::stringstream sstr(valueStr);
			double val;
			sstr >> val;
			return val;
		}
		else
		{
			return 0.0;
		}
	}
	bool Value::ToBool() const
	{
		if (IsBool())
		{
			return (valueStr == "true");
		}
		else
		{
			return false;
		}
	}

	void Value::SetNull()
	{
		valueStr = "";
		type     = VT_NULL;
	}
	void Value::Set(const Value &value)
	{
		if (this != &value)
		{
			valueStr = value.valueStr;
			type     = value.type;
		}
	}
	void Value::Set(ValueType type, const std::string &value)
	{
		valueStr   = value;
		this->type = type;
	}
	void Value::Set(const std::string &value)
	{
		valueStr = UnescapeString(value);
		type     = VT_STRING;
	}
	void Value::Set(const char *value)
	{
		valueStr = UnescapeString(std::string(value));
		type     = VT_STRING;
	}
	void Value::Set(const int value)
	{
		std::stringstream sstr;
		sstr << value;
		valueStr = sstr.str();
		type     = VT_NUMBER;
	}
	void Value::Set(const float value)
	{
		std::stringstream sstr;
		sstr << value;
		valueStr = sstr.str();
		type     = VT_NUMBER;
	}
	void Value::Set(const double value)
	{
		std::stringstream sstr;
		sstr << value;
		valueStr = sstr.str();
		type     = VT_NUMBER;
	}
	void Value::Set(const bool value)
	{
		if (value)
			valueStr = "true";
		else
			valueStr = "false";
		type = VT_BOOL;
	}

	Value &Value::operator=(const Value &rhs)
	{
		if (this != &rhs)
			Set(rhs);
		return *this;
	}
	Value &Value::operator=(const Node &rhs)
	{
		if (this != &rhs)
			Set(rhs.AsValue());
		return *this;
	}
	Value &Value::operator=(const std::string &rhs)
	{
		Set(rhs);
		return *this;
	}
	Value &Value::operator=(const char *rhs)
	{
		Set(rhs);
		return *this;
	}
	Value &Value::operator=(const int rhs)
	{
		Set(rhs);
		return *this;
	}
	Value &Value::operator=(const float rhs)
	{
		Set(rhs);
		return *this;
	}
	Value &Value::operator=(const double rhs)
	{
		Set(rhs);
		return *this;
	}
	Value &Value::operator=(const bool rhs)
	{
		Set(rhs);
		return *this;
	}

	bool Value::operator==(const Value &other) const
	{
		return ((type == other.type)&&(valueStr == other.valueStr));
	}
	bool Value::operator!=(const Value &other) const
	{
		return !(*this == other);
	}

	Node *Value::GetCopy() const
	{
		return new Value(*this);
	}

	// This is not the most beautiful place for these, but it'll do
	static const char charsUnescaped[] = { '\\'  , '/'  , '\"'  , '\n' , '\t' , '\b' , '\f' , '\r' };
	static const char *charsEscaped[]  = { "\\\\", "\\/", "\\\"", "\\n", "\\t", "\\b", "\\f", "\\r" };
	static const unsigned int numEscapeChars = 8;
	static const char nullUnescaped = '\0';
	static const char *nullEscaped  = "\0\0";
	const char *&getEscaped(const char &c)
	{
		for (unsigned int i = 0; i < numEscapeChars; ++i)
		{
			const char &ue = charsUnescaped[i];

			if (c == ue)
			{
				const char *&e = charsEscaped[i];
				return e;
			}
		}
		return nullEscaped;
	}
	const char &getUnescaped(const char &c1, const char &c2)
	{
		for (unsigned int i = 0; i < numEscapeChars; ++i)
		{
			const char *&e = charsEscaped[i];

			if (c1 == e[0] && c2 == e[1])
			{
				const char &ue = charsUnescaped[i];
				return ue;
			}
		}
		return nullUnescaped;
	}

	std::string Value::EscapeString(const std::string &value)
	{
		std::string escaped;

		for (std::string::const_iterator it = value.begin(); it != value.end(); ++it)
		{
			const char &c = (*it);

			const char *&a = getEscaped(c);
			if (a[0] != '\0')
			{
				escaped += a[0];
				escaped += a[1];
			}
			else
			{
				escaped += c;
			}
		}

		return escaped;
	}
	std::string Value::UnescapeString(const std::string &value)
	{
		std::string unescaped;

		for (std::string::const_iterator it = value.begin(); it != value.end(); ++it)
		{
			const char &c = (*it);
			char c2 = '\0';
			if (it+1 != value.end())
				c2 = *(it+1);

			const char &a = getUnescaped(c, c2);
			if (a != '\0')
			{
				unescaped += a;
				if (it+1 != value.end())
					++it;
			}
			else
			{
				unescaped += c;
			}
		}

		return unescaped;
	}


	Object::Object() : Node()
	{
	}
	Object::Object(const Object &other) : Node()
	{
		for (ChildList::const_iterator it = other.children.begin(); it != other.children.end(); ++it)
		{
			const std::string &name = (*it).first;
			Node &value = *(*it).second;

			children.push_back(NamedNodePtr(name, value.GetCopy()));
		}
	}
	Object::Object(const Node &other) : Node()
	{
		const Object &object = other.AsObject();

		for (ChildList::const_iterator it = object.children.begin(); it != object.children.end(); ++it)
		{
			const std::string &name = (*it).first;
			Node &value = *(*it).second;

			children.push_back(NamedNodePtr(name, value.GetCopy()));
		}
	}
	Object::~Object()
	{
		Clear();
	}

	Node::Type Object::GetType() const
	{
		return T_OBJECT;
	}

	void Object::Add(const std::string &name, Node &node)
	{
		children.push_back(NamedNodePtr(name, node.GetCopy()));
	}
	void Object::Add(const std::string &name, Value node)
	{
		children.push_back(NamedNodePtr(name, new Value(node)));
	}
	void Object::Remove(const std::string &name)
	{
		for (ChildList::iterator it = children.begin(); it != children.end(); ++it)
		{
			if ((*it).first == name)
			{
				delete (*it).second;
				children.erase(it);
				break;
			}
		}
	}
	void Object::Clear()
	{
		for (ChildList::iterator it = children.begin(); it != children.end(); ++it)
		{
			delete (*it).second;
			(*it).second = NULL;
		}
		children.clear();
	}

	Object::iterator Object::begin()
	{
		if (!children.empty())
			return Object::iterator(&children.front());
		else
			return Object::iterator(NULL);
	}
	Object::const_iterator Object::begin() const
	{
		if (!children.empty())
			return Object::const_iterator(&children.front());
		else
			return Object::const_iterator(NULL);
	}
	Object::iterator Object::end()
	{
		if (!children.empty())
			return Object::iterator(&children.back()+1);
		else
			return Object::iterator(NULL);
	}
	Object::const_iterator Object::end() const
	{
		if (!children.empty())
			return Object::const_iterator(&children.back()+1);
		else
			return Object::const_iterator(NULL);
	}

	bool Object::Has(const std::string &name) const
	{
		for (ChildList::const_iterator it = children.begin(); it != children.end(); ++it)
		{
			if ((*it).first == name)
			{
				return true;
			}
		}
		return false;
	}
	size_t Object::GetCount() const
	{
		return children.size();
	}
	Node &Object::Get(const std::string &name) const
	{
		for (ChildList::const_iterator it = children.begin(); it != children.end(); ++it)
		{
			if ((*it).first == name)
			{
				return *(*it).second;
			}
		}

		throw NotFoundException();
	}

	Node *Object::GetCopy() const
	{
		return new Object(*this);
	}


	Array::Array() : Node()
	{
	}
	Array::Array(const Array &other) : Node()
	{
		for (ChildList::const_iterator it = other.children.begin(); it != other.children.end(); ++it)
		{
			const Node &value = *(*it);

			children.push_back(value.GetCopy());
		}
	}
	Array::Array(const Node &other) : Node()
	{
		const Array &array = other.AsArray();

		for (ChildList::const_iterator it = array.children.begin(); it != array.children.end(); ++it)
		{
			const Node &value = *(*it);

			children.push_back(value.GetCopy());
		}
	}
	Array::~Array()
	{
		Clear();
	}

	Node::Type Array::GetType() const
	{
		return T_ARRAY;
	}

	void Array::Add(Node &node)
	{
		children.push_back(node.GetCopy());
	}
	void Array::Add(Value node)
	{
		children.push_back(new Value(node));
	}
	void Array::Remove(size_t index)
	{
		if (index < children.size())
		{
			ChildList::iterator it = children.begin()+index;
			delete (*it);
			children.erase(it);
		}
	}
	void Array::Clear()
	{
		for (ChildList::iterator it = children.begin(); it != children.end(); ++it)
		{
			delete (*it);
			(*it) = NULL;
		}
		children.clear();
	}

	Array::iterator Array::begin()
	{
		if (!children.empty())
			return Array::iterator(&children.front());
		else
			return Array::iterator(NULL);
	}
	Array::const_iterator Array::begin() const
	{
		if (!children.empty())
			return Array::const_iterator(&children.front());
		else
			return Array::const_iterator(NULL);
	}
	Array::iterator Array::end()
	{
		if (!children.empty())
			return Array::iterator(&children.back()+1);
		else
			return Array::iterator(NULL);
	}
	Array::const_iterator Array::end() const
	{
		if (!children.empty())
			return Array::const_iterator(&children.back()+1);
		else
			return Array::const_iterator(NULL);
	}

	size_t Array::GetCount() const
	{
		return children.size();
	}
	Node &Array::Get(size_t index) const
	{
		if (index < children.size())
		{
			return *children.at(index);
		}

		throw NotFoundException();
	}

	Node *Array::GetCopy() const
	{
		return new Array(*this);
	}


	FileWriter::FileWriter(const std::string &filename) : filename(filename)
	{
	}
	FileWriter::~FileWriter()
	{
	}

	void FileWriter::WriteFile(const std::string &filename, const Node &root, const Format &format)
	{
		FileWriter writer(filename);
		writer.Write(root, format);
	}

	void FileWriter::Write(const Node &root, const Format &format)
	{
		Writer writer(root, format);
		writer.Write();

		std::fstream file(filename.c_str(), std::ios::out | std::ios::trunc);
		file << writer.GetResult();
		file.close();
	}


	FileReader::FileReader(const std::string &filename)
	{
		if (!loadFile(filename, json))
		{
			error = "Failed to load file";
		}
	}
	FileReader::~FileReader()
	{
	}

	bool FileReader::ReadFile(const std::string &filename, Node &node)
	{
		FileReader reader(filename);
		return reader.Read(node);
	}

	bool FileReader::Read(Node &node)
	{
		if (!error.empty())
			return false;

		Parser parser(node, json);
		if (!parser.Parse())
		{
			error = parser.GetError();
			return false;
		}
		else
		{
			return true;
		}
	}

	Node::Type FileReader::DetermineType()
	{
		return Node::DetermineType(json);
	}

	const std::string &FileReader::GetError() const
	{
		return error;
	}

	bool FileReader::loadFile(const std::string &filename, std::string &json)
	{
		std::fstream file(filename.c_str(), std::ios::in | std::ios::binary);

		if (!file.is_open())
		{
			return false;
		}

		file.seekg(0, std::ios::end);
		std::ios::pos_type size = file.tellg();
		file.seekg(0, std::ios::beg);

		json.resize(static_cast<std::string::size_type>(size), '\0');
		file.read(&json[0], size);

		return true;
	}


	Writer::Writer(const Node &root, const Format &format) : fi(new FormatInterpreter), root(root)
	{
		SetFormat(format);
	}
	Writer::~Writer()
	{
		delete fi;
		fi = NULL;
	}

	void Writer::SetFormat(const Format &format)
	{
		fi->SetFormat(format);
	}
	const std::string &Writer::Write()
	{
		result.clear();
		writeNode(root, 0);
		return result;
	}

	const std::string &Writer::GetResult() const
	{
		return result;
	}

	void Writer::writeNode(const Node &node, unsigned int level)
	{
		switch (node.GetType())
		{
		case Node::T_OBJECT : writeObject(node.AsObject(), level); break;
		case Node::T_ARRAY  : writeArray(node.AsArray(), level);   break;
		case Node::T_VALUE  : writeValue(node.AsValue());          break;
		}
	}
	void Writer::writeObject(const Object &node, unsigned int level)
	{
		result += "{" + fi->GetNewline();

		for (Object::const_iterator it = node.begin(); it != node.end(); ++it)
		{
			const std::string &name = (*it).first;
			// const Node &value = (*it).second;

			if (it != node.begin())
				result += "," + fi->GetNewline();
			result += fi->GetIndentation(level+1) + "\""+name+"\"" + ":" + fi->GetSpacing();
			writeNode((*it).second, level+1);
		}

		result += fi->GetNewline() + fi->GetIndentation(level) + "}";
	}
	void Writer::writeArray(const Array &node, unsigned int level)
	{
		result += "[" + fi->GetNewline();

		for (Array::const_iterator it = node.begin(); it != node.end(); ++it)
		{
			const Node &value = (*it);

			if (it != node.begin())
				result += "," + fi->GetNewline();
			result += fi->GetIndentation(level+1);
			writeNode(value, level+1);
		}

		result += fi->GetNewline() + fi->GetIndentation(level) + "]";
	}
	void Writer::writeValue(const Value &node)
	{
		if (node.IsString())
		{
			result += "\""+Value::EscapeString(node.ToString())+"\"";
		}
		else
		{
			result += node.ToString();
		}
	}


	Parser::Parser(Node &root) : jsonSize(0), cursor(0), root(root)
	{
	}
	Parser::Parser(Node &root, const std::string &json) : jsonSize(0), cursor(0), root(root)
	{
		SetJson(json);
	}
	Parser::~Parser()
	{
	}

	void Parser::SetJson(const std::string &json)
	{
		this->json = json;
		jsonSize   = json.size();
	}
	bool Parser::Parse()
	{
		cursor = 0;

		tokenize();
		bool success = assemble();

		return success;
	}

	const std::string &Parser::GetError() const
	{
		return error;
	}

	void Parser::tokenize()
	{
		Token token;
		std::string valueBuffer;
		bool saveBuffer;

		char c = '\0';
		for (; cursor < jsonSize; ++cursor)
		{
			c = json.at(cursor);

			if (IsWhitespace(c))
				continue;

			saveBuffer = true;

			switch (c)
			{
			case '{' :
				{
					token = T_OBJ_BEGIN;
					break;
				}
			case '}' :
				{
					token = T_OBJ_END;
					break;
				}
			case '[' :
				{
					token = T_ARRAY_BEGIN;
					break;
				}
			case ']' :
				{
					token = T_ARRAY_END;
					break;
				}
			case ',' :
				{
					token = T_SEPARATOR_NODE;
					break;
				}
			case ':' :
				{
					token = T_SEPARATOR_NAME;
					break;
				}
			case '"' :
				{
					token = T_VALUE;
					readString();
					break;
				}
			case '/' :
				{
					char p = peek();
					if (p == '*')
					{
						jumpToCommentEnd();
					}
					else if (p == '/')
					{
						jumpToNext('\n');
					}
					break;
				}
			default :
				{
					valueBuffer += c;
					saveBuffer = false;
					break;
				}
			}

			if ((saveBuffer || cursor == jsonSize-1) && (!valueBuffer.empty())) // Always save buffer on the last character
			{
				if (interpretValue(valueBuffer))
				{
					tokens.push(T_VALUE);
				}
				else
				{
					// Store the unknown token, so we can show it to the user
					data.push(MakePair(Value::VT_STRING, valueBuffer));
					tokens.push(T_UNKNOWN);
				}

				valueBuffer.clear();
			}

			// Push the token last so that any
			// value token will get pushed first
			// from above.
			// If saveBuffer is false, it means that
			// we are in the middle of a value, so we
			// don't want to push any tokens now.
			if (saveBuffer)
			{
				tokens.push(token);
			}
		}
	}
	bool Parser::assemble()
	{
		std::stack<Pair<std::string, Node*> > nodeStack;

		std::string name = "";

		Token token;
		while (!tokens.empty())
		{
			token = tokens.front();
			tokens.pop();

			switch (token)
			{
			case T_UNKNOWN :
				{
					const std::string &unknownToken = data.front().second;
					error = "Unknown token: "+unknownToken;
					data.pop();
					return false;
				}
			case T_OBJ_BEGIN :
				{
					Node *node = NULL;
					if (nodeStack.empty())
					{
						if (!root.IsObject())
						{
							error = "The given root node is not an object";
							return false;
						}

						node = &root;
					}
					else
					{
						node = new Object;
					}

					nodeStack.push(MakePair(name, node));
					name.clear();
					break;
				}
			case T_ARRAY_BEGIN :
				{
					Node *node = NULL;
					if (nodeStack.empty())
					{
						if (!root.IsArray())
						{
							error = "The given root node is not an array";
							return false;
						}

						node = &root;
					}
					else
					{
						node = new Array;
					}

					nodeStack.push(MakePair(name, node));
					name.clear();
					break;
				}
			case T_OBJ_END :
			case T_ARRAY_END :
				{
					if (nodeStack.empty())
					{
						error = "Found end of object or array without beginning";
						return false;
					}
					if (token == T_OBJ_END && !nodeStack.top().second->IsObject())
					{
						error = "Mismatched end and beginning of object";
						return false;
					}
					if (token == T_ARRAY_END && !nodeStack.top().second->IsArray())
					{
						error = "Mismatched end and beginning of array";
						return false;
					}

					std::string name = nodeStack.top().first;
					Node *node = nodeStack.top().second;
					nodeStack.pop();

					if (!nodeStack.empty())
					{
						if (nodeStack.top().second->IsObject())
						{
							nodeStack.top().second->AsObject().Add(name, *node);
						}
						else if (nodeStack.top().second->IsArray())
						{
							nodeStack.top().second->AsArray().Add(*node);
						}
						else
						{
							error = "Can only add elements to objects and arrays";
							return false;
						}

						delete node;
						node = NULL;
					}
					break;
				}
			case T_VALUE :
				{
					if (!tokens.empty() && tokens.front() == T_SEPARATOR_NAME)
					{
						tokens.pop();
						if (data.front().first != Value::VT_STRING)
						{
							error = "A name has to be a string";
							return false;
						}
						else
						{
							name = data.front().second;
							data.pop();
						}
					}
					else
					{
						Node *node = NULL;
						if (nodeStack.empty())
						{
							if (!root.IsValue())
							{
								error = "The given root node is not a value";
								return false;
							}

							node = &root;
						}
						else
						{
							node = new Value;
						}

						if (data.front().first == Value::VT_STRING)
						{
							static_cast<Value*>(node)->Set(data.front().second); // This method calls UnescapeString()
						}
						else
						{
							static_cast<Value*>(node)->Set(data.front().first, data.front().second);
						}
						data.pop();

						if (!nodeStack.empty())
						{
							if (nodeStack.top().second->IsObject())
								nodeStack.top().second->AsObject().Add(name, *node);
							else if (nodeStack.top().second->IsArray())
								nodeStack.top().second->AsArray().Add(*node);

							delete node;
							node = NULL;
							name.clear();
						}
						else
						{
							nodeStack.push(MakePair(name, node));
							name.clear();
						}
					}
					break;
				}
			case T_SEPARATOR_NAME :
			case T_SEPARATOR_NODE : break;
			}
		}

		return true;
	}

	char Parser::peek()
	{
		if (cursor < jsonSize-1)
		{
			return json.at(cursor+1);
		}
		else
		{
			return '\0';
		}
	}
	void Parser::jumpToNext(char c)
	{
		++cursor;
		while (cursor < jsonSize && json.at(cursor) != c)
			++cursor;
	}
	void Parser::jumpToCommentEnd()
	{
		++cursor;
		char c1 = '\0', c2 = '\0';
		for (; cursor < jsonSize; ++cursor)
		{
			c2 = json.at(cursor);

			if (c1 == '*' && c2 == '/')
				break;

			c1 = c2;
		}
	}

	void Parser::readString()
	{
		if (json.at(cursor) != '"')
			return;

		std::string str;

		++cursor;

		char c1 = '\0', c2 = '\0';
		for (; cursor < jsonSize; ++cursor)
		{
			c2 = json.at(cursor);

			if (c1 != '\\' && c2 == '"')
			{
				break;
			}

			str += c2;

			c1 = c2;
		}

		data.push(MakePair(Value::VT_STRING, str));
	}
	bool Parser::interpretValue(const std::string &value)
	{
		std::string upperValue(value.size(), '\0');

		std::transform(value.begin(), value.end(), upperValue.begin(), toupper);

		if (upperValue == "NULL")
		{
			data.push(MakePair(Value::VT_NULL, std::string("")));
		}
		else if (upperValue == "TRUE")
		{
			data.push(MakePair(Value::VT_BOOL, std::string("true")));
		}
		else if (upperValue == "FALSE")
		{
			data.push(MakePair(Value::VT_BOOL, std::string("false")));
		}
		else
		{
			bool number = true;
			for (std::string::const_iterator it = value.begin(); it != value.end(); ++it)
			{
				if (!IsNumber(*it))
				{
					number = false;
					break;
				}
			}

			if (number)
			{
				data.push(MakePair(Value::VT_NUMBER, value));
			}
			else
			{
				return false;
			}
		}

		return true;
	}
}
