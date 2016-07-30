/* This file is part of the Springlobby (GPL v2 or later), see COPYING */

#include "tdfcontainer.h"

#include <lslutils/conversion.h>
#include <lslutils/autopointers.h>
#include <lslutils/debug.h>

#include <cmath>
#include <sstream>
#include <fstream>
#include <iomanip>
#include <algorithm>
#include <boost/algorithm/string.hpp>

#define ASSERT_LOGIC(...) \
	do {              \
	} while (0)

namespace LSL
{
namespace TDF
{

inline bool IsWhitespace(char c)
{
	return (c == ' ') || (c == 10) || (c == 13) || (c == '\t');
}

/** \brief Tokenizer used in TDF parsing
 * \todo clean up
 **/
class Tokenizer
{

	/// simple reference counted pointer to stream.
	struct IncludeCacheEntry
	{
		std::string name; ///< used for error reporting
		int line;
		int column;
		std::istream* stream;
		//bool must_delete;
		int* refcount;

		IncludeCacheEntry(std::istream* stream_, bool must_delete_ = false)
		    : line(1)
		    , column(1)
		    , stream(stream_)
		    , refcount(NULL)
		{
			if (must_delete_) {
				refcount = new int;
				(*refcount) = 1;
			}
		}
		IncludeCacheEntry(const IncludeCacheEntry& other)
		    : line(other.line)
		    , column(other.column)
		    , stream(other.stream)
		    , refcount(other.refcount)
		{
			stream = other.stream;
			refcount = other.refcount;
			if (refcount)
				(*refcount) += 1;
		}
		~IncludeCacheEntry()
		{
			if (refcount) {
				(*refcount)--;
				if ((*refcount) <= 0) {
					delete stream;
					delete refcount;
				}
			}
		}
	};
	std::vector<IncludeCacheEntry> include_stack;
	void UnwindStack();

	std::deque<Token> token_buffer;
	//size_t max_buffer_size;
	size_t buffer_pos;

	bool skip_eol;
	char GetNextChar();
	char PeekNextChar();

	void ReadToken(Token& token);
	void SkipSpaces();

	int errors;

public:
	Tokenizer()
	    : buffer_pos(0)
	    , skip_eol(false)
	    , errors(0)
	{
	}

	void EnterStream(std::istream& stream_, const std::string& name = "");

	Token GetToken(int i = 0);
	void Step(int i = 1);
	inline Token TakeToken()
	{
		Token result = GetToken();
		Step();
		return result;
	}

	bool Good();

	void ReportError(const Token& t, const std::string& err);

	int NumErrors() const
	{
		return errors;
	}
};

inline Tokenizer& operator>>(Tokenizer& tokenizer, Token& token)
{
	token = tokenizer.TakeToken();
	return tokenizer;
}

namespace BA = boost::algorithm;

TDFWriter::TDFWriter(std::stringstream& s)
    : m_stream(s)
    , m_depth(0)
{
}

TDFWriter::~TDFWriter()
{
	Close();
}
void TDFWriter::EnterSection(const std::string& name)
{
	Indent();
	m_stream << "[" << name << "]\n";
	Indent();
	m_stream << "{\n";
	m_depth++;
}
void TDFWriter::LeaveSection()
{
	m_depth--;
	Indent();
	m_stream << "}\n";
}
void TDFWriter::Indent()
{
	for (int i = 0; i < m_depth; ++i)
		m_stream << "\t";
}

void TDFWriter::AppendStr(const std::string& name, std::string value)
{
	Indent();
	m_stream << name << "=" << value << ";\n";
}

void TDFWriter::AppendInt(const std::string& name, int value)
{
	AppendStr(name, Util::ToIntString(value));
}

void TDFWriter::AppendFloat(const std::string& name, float value)
{
	AppendStr(name, Util::ToFloatString(value));
}


void TDFWriter::Close()
{
	while (m_depth > 0)
		LeaveSection();
	if (m_depth < 0) {
		LslError("error in TDFWriter usage: more LeaveSection() calls than EnterSection(). Please contact springlobby developers");
	}
}

void TDFWriter::AppendLineBreak()
{
	m_stream << "\n";
}

void Tokenizer::ReportError(const Token& t, const std::string& err)
{
	LslError("TDF parsing error at (%s), on token \"%s\" : %s", t.pos_string.c_str(), t.value_s.c_str(), err.c_str());
	errors++;
}

void Tokenizer::EnterStream(std::istream& stream_, const std::string& name)
{
	skip_eol = false;
	include_stack.push_back(IncludeCacheEntry(&stream_, false));
	include_stack.back().name = name;
}

void Tokenizer::UnwindStack()
{
	while ((!include_stack.empty()) && include_stack.back().stream && (!include_stack.back().stream->good())) {
		include_stack.pop_back();
	}
}

char Tokenizer::PeekNextChar()
{
	UnwindStack();
	//std::string tmp;
	char tmp = 0;
	if (!include_stack.empty())
		tmp = (*include_stack.back().stream).peek();
	return tmp;
}

char Tokenizer::GetNextChar()
{
	UnwindStack();
	//std::string tmp;
	if (include_stack.empty())
		return 0;

	char c = (*include_stack.back().stream).get();

	if ((!skip_eol) && (c == 10 || c == 13)) { // end of line
		include_stack.back().line += 1;
		include_stack.back().column = 1;

		// if next is different we'll skip it.
		//std::istream &stream=(*include_stack.back().stream);
		char nc = (*include_stack.back().stream).peek();
		if ((nc == 10 || nc == 13) && (nc != c))
			skip_eol = true;
	} else {
		if (!skip_eol)
			include_stack.back().column += 1;
		skip_eol = false;
	}

	return c;
}

bool Tokenizer::Good()
{
	UnwindStack();
	return (errors == 0) && !include_stack.empty();
}

void Tokenizer::ReadToken(Token& token)
{
start:

	SkipSpaces();
	token.value_s.clear();

	if (!Good()) {
		token.type = Token::type_eof;
		token.pos_string = "EOF";
		return;
	}

	std::stringstream token_tmp;
	if (!include_stack.empty() && !include_stack.back().name.empty())
		token_tmp << include_stack.back().name << " , ";
	token_tmp << "line " << include_stack.back().line << " , column " << include_stack.back().column;
	token.pos_string = token_tmp.str();

	char c = GetNextChar();
	token.value_s += c;
	// first find what token is it, and handle all except numbers
	switch (c) {
		case '[': {
			token.type = Token::type_section_name;
			token.value_s.clear();
			bool skip_next_eol_char = false;
			while (Good()) {
				c = GetNextChar();
				// std::string has problem with zero characters, replace by space.
				if (c == 0) {
					c = ' ';
				}
				if (c == '\\') {
					if (Good()) {
						token.value_s += GetNextChar();
					} else {
						ReportError(token, "Quotes not closed before end of file");
						return;
					}
				} else if (c == ']') {
					return;
				} else {
					token.value_s += c;
				}
				// handle end of line
				if (skip_next_eol_char) {
					skip_next_eol_char = false;
				} else if (c == 10 || c == 13) {
					//++line;
					//column=1;
					if ((PeekNextChar() == 10 || PeekNextChar() == 13) && (PeekNextChar() != c))
						skip_next_eol_char = true;
				}
			}
			ReportError(token, "Quotes not closed before end of file");
			return;
		}
		case '{':
			token.type = Token::type_enter_section;
			return;
		case '}':
			token.type = Token::type_leave_section;
			return;
		case ';':
			token.type = Token::type_semicolon;
			return;
		case '=':
			token.type = Token::type_entry_value;
			token.value_s = "";
			while (Good() && PeekNextChar() != ';') {
				unsigned char c_ = GetNextChar();
				token.value_s += c_;
			}
			return;
		case '/': // handle comments
			if (PeekNextChar() == '/') {
				//SkipToEOL();
				if (!include_stack.empty()) {
					std::string tmp;
					std::getline((*include_stack.back().stream), tmp);
					include_stack.back().line += 1;
					include_stack.back().column = 1;
				}

				goto start;
			} else if (PeekNextChar() == '*') { // multi-line comment
				while (Good()) {
					char c1 = GetNextChar();
					if ((c1 == '*') && (PeekNextChar() == '/')) {
						GetNextChar();
						break;
					}
				}
				goto start;
			}
		default:
			while (Good() && PeekNextChar() != '=') {
				unsigned char c_ = GetNextChar();
				token.value_s += c_;
			}
			token.type = Token::type_entry_name;
			return;
	}
}

void Tokenizer::SkipSpaces()
{
	while (Good() && IsWhitespace(PeekNextChar())) {
		GetNextChar();
	}
}

Token Tokenizer::GetToken(int i)
{
	int p = buffer_pos + i;
	if (p < 0)
		return Token();
	while (int(token_buffer.size()) < p + 1) {
		Token t;
		ReadToken(t);
		if (t.IsEOF())
			return t;
		token_buffer.push_back(t);
	}

	return token_buffer[p];
}

void Tokenizer::Step(int i)
{
	buffer_pos += i;
}

Node::~Node()
{
	//if(parent)parent->Remove(name);
}

DataList* Node::Parent() const
{ // parent list
	return parent;
}

bool Node::IsChildOf(DataList* what) const
{
	DataList* current = Parent();
	while (current) {
		if (current == what)
			return true;
		current = current->Parent();
	}
	return false;
}

void Node::ListRemove()
{
	//if(parent->list_first==this)parent->list_first=next;
	//if(parent->list_last==this)parent->list_last=prev;
	if (list_prev) {
		list_prev->list_next = this->list_next;
	}
	if (list_next) {
		list_next->list_prev = this->list_prev;
	}
	list_prev = NULL;
	list_next = NULL;
}

void Node::ListInsertAfter(Node* what)
{
	ListRemove();
	if (!what)
		return;
	this->list_next = what->list_next;
	if (what->list_next)
		what->list_next->list_prev = this;
	what->list_next = this;
	this->list_prev = what;
}

std::string Node::Name()
{
	return name;
}
bool Node::SetName(const std::string& name_)
{
	if (parent) {
		return parent->Rename(name, name_);
	}
	name = name_;
	return true;
}

void Node::Save(TDFWriter& /*unused*/)
{
	/// nothing to save there.
}
void Node::Load(Tokenizer& /*unused*/)
{
	/// nothing to load there.
	//ASSERT_LOGIC(0,\1;
}

/// ***********************************************************
///  class DataList
/// ***********************************************************

DataList::DataList()
{
	//parent = NULL;
	list_loop.list_prev = &list_loop;
	list_loop.list_next = &list_loop;
	list_loop.parent = this;
	list_loop.Reference();
}

DataList::~DataList()
{ // disconnect from childs
	TDF::PNode node = First();
	while (node.Ok() && node != End()) {
		//std::cout<<"printing"<<std::endl;
		TDF::PNode nextnode = Next(node);
		node->parent = NULL;
		node->ListRemove();
		node = nextnode;
	}
}

bool DataList::Insert(PNode node) /// return false if such entry already exists.
{
	if (!node.Ok())
		return false;
	bool inserted = nodes.insert(std::pair<std::string, PNode>(BA::to_lower_copy(node->name), node)).second;
	if (!inserted)
		return false;

	node->parent = this;
	node->ListInsertAfter(list_loop.list_prev);
	return true;
}

bool DataList::InsertAt(PNode node, PNode where) /// return false if such entry already exists.
{
	if (!node.Ok())
		return false;
	if (!(where->list_prev))
		return false;
	bool inserted = nodes.insert(std::pair<std::string, PNode>((*node).name, node)).second;
	if (!inserted)
		return false;

	node->parent = this;
	node->ListInsertAfter(where->list_prev);
	return true;
}

static const char* rename_prefix = "!";

void DataList::InsertRename(PNode node)
{ /// rename if such entry already exists. str contains new name.
	if (!node.Ok())
		return;

	if (!Insert(node)) {
		std::string original_name = node->Name();
		for (int n = 0; n < 10000; ++n) {
			//std::string tmp=str+std::string(rename_prefix);
			std::ostringstream os;
			os << original_name << rename_prefix << n;
			node->name = os.str();
			if (Insert(node)) {
				return;
			}
		}
		LslError("insertRename: iterated over 10 000 names, way too many");
	}
}

void DataList::InsertRenameAt(PNode node, PNode where)
{ // rename if such entry already exists. str contains new name.
	if (!node.Ok())
		return;
	if (!where->list_prev)
		return;

	if (!InsertAt(node, where)) {
		for (int n = 0; n < 10000; ++n) {
			std::ostringstream os;
			os << node->Name() << rename_prefix << n;
			node->name = os.str();
			if (InsertAt(node, where)) {
				return;
			}
		}
		LslError("insertRename: iterated over 10 000 names, way too many");
	}
}

bool DataList::Remove(const std::string& str)
{
	//PNode node=nodes.find(str.Lower())->last;
	PNode node = Find(str);
	if (!node.Ok())
		return false;
	if (nodes.erase(BA::to_lower_copy(str)) <= 0)
		return false;

	node->parent = NULL;
	node->ListRemove();
	return true;
}

bool DataList::Remove(PNode node)
{
	if (!node.Ok())
		return false;
	if (nodes.erase(BA::to_lower_copy(node->Name())) <= 0)
		return false;

	node->parent = NULL;
	node->ListRemove();
	return true;
}

bool DataList::Rename(const std::string& old_name, const std::string& new_name)
{
	// check that new name is not used up.
	if (nodes.find(BA::to_lower_copy(new_name)) != nodes.end())
		return false;
	nodes_iterator i = nodes.find(BA::to_lower_copy(old_name));
	if (i == nodes.end())
		return false;
	PNode node = i->second;

	ASSERT_LOGIC(node.Ok(), "Internal TDF tree consistency (1)");
	ASSERT_LOGIC(node->Name().Lower() == old_name.Lower(), "Internal TDF tree consistency (2)");

	node->name = BA::to_lower_copy(new_name);
	nodes.erase(i);
	bool inserted = nodes.insert(std::pair<std::string, PNode>(BA::to_lower_copy(node->name), node)).second;
	ASSERT_LOGIC(inserted, "DataList::Rename failed");
	return inserted;
}

/// find by name. unused.
PNode DataList::Find(const std::string& str)
{
	if (str == "..")
		return Parent();
	if (str == ".")
		return this;
	nodes_iterator i = nodes.find(BA::to_lower_copy(str));
	if (i != nodes.end()) {
		ASSERT_LOGIC(BA::to_lower_copy(i->second->Name()) == BA::to_lower_copy(str), "Internal TDF tree consistency (3)");
		return i->second;
	}
	return NULL;
}

std::string DataList::Path()
{
	/*
	std::string result; //FIXME: is not used?!
	PDataList tmp( this );
	while ( tmp.Ok() ) {
		result = std::string( "/" ) + tmp->Name();
		tmp = tmp->Parent();
	}
*/
	return name;
}

PNode DataList::FindByPath(const std::string& str)
{
	if (str.empty())
		return this;
	int i = 0;
	std::string buff;
	PDataList current_dir(this);
	if (str[i] == '/') { // go to root
		PDataList tmp = Parent();
		while (tmp.Ok()) {
			current_dir = tmp;
			tmp = tmp->Parent();
		}
	} else {
		buff += str[0];
	}
	i = 1;
	while ((unsigned int)(i) < str.size()) {
		if (str[i] == '/') {
			if (buff == "..") {
				current_dir = current_dir->Parent();
				if (!current_dir.Ok())
					return NULL;
			} else if (buff != "." && !buff.empty()) { //
				PNode node = current_dir->Find(buff);
				if (!node.Ok())
					return NULL;
				PDataList datalist(node);
				if (datalist.Ok()) {
					current_dir = datalist;
				} else
					return NULL;
			}
			buff = "";
		} else {
			buff += str[i];
		}
		++i;
	}
	if (current_dir.Ok()) {
		if (!buff.empty()) {
			return current_dir->Find(buff);
		} else
			return PNode(current_dir);
	} else {
		return NULL;
	}
}

PNode DataList::Next(PNode what)
{
	if (what.Ok())
		return what->list_next;
	return NULL;
}
PNode DataList::Prev(PNode what)
{
	if (what.Ok())
		return what->list_prev;
	return NULL;
}
PNode DataList::End()
{
	return PNode(&list_loop);
}
PNode DataList::First()
{
	return PNode(list_loop.list_next);
}
PNode DataList::Last()
{
	return PNode(list_loop.list_prev);
}

/*
void DataList::PrintContent(std::ostream &s) {
    PNode node=First();

    while(node.Ok() && node!=End()){
      //std::cout<<"printing"<<std::endl;
      #ifdef use_std_string
      s<<std::endl<<"'"<<node->Name()<<"'={";
        node->PrintContent(s);
        s<<"}"<<std::endl;
      #else
        s<<std::endl<<"'"<<node->Name().mb_str()<<"'={";
        node->PrintContent(s);
        s<<"}"<<std::endl;
      #endif
      node=Next(node);
    }
}*/
void DataList::Save(TDFWriter& f)
{
	PNode node = First();
	if (node == End())
		return;

	while (node.Ok() && node != End()) {
		// if class name is not set properly, continue
		//if(node->ClassName().empty())continue;
		PDataList list(node);
		if (list.Ok()) {
			f.EnterSection(list->Name());
			list->Save(f);
			f.LeaveSection();
		} else {
			PDataLeaf leaf(node);
			if (leaf.Ok()) {
				leaf->Save(f);
			}
		}
		node = Next(node);
	}
}

void DataList::Load(Tokenizer& f)
{
	while (f.Good()) {
		Token t = f.TakeToken();
		switch (t.type) {
			case Token::type_leave_section:
			case Token::type_eof:
				return;
			case Token::type_entry_name: {
				PDataLeaf new_leaf(new DataLeaf);
				new_leaf->SetName(t.value_s);
				new_leaf->Load(f);
				Insert(PNode(new_leaf));
			} break;

			case Token::type_section_name: {
				Token t2 = f.TakeToken();
				if (t2.type != Token::type_enter_section) {
					f.ReportError(t, "'{' expected");
				} else {
					PDataList new_list(new DataList);
					new_list->SetName(t.value_s);
					new_list->Load(f); // will eat the '}'
					Insert(PNode(new_list));
				}
			} break;
			default:
				f.ReportError(t, "[sectionname] or entryname= expected.");
		}
	}
}


int DataList::GetInt(const std::string& f_name, int default_value, bool* it_worked)
{
	PDataLeaf leaf(Find(f_name));
	if (!leaf.ok()) {
		if (it_worked) {
			*it_worked = false;
		}
		return default_value;
	}
	std::string s = leaf->GetValue();
	long result = Util::FromIntString(s);
	if (it_worked) {
		*it_worked = true;
	}
	return result;
}
double DataList::GetDouble(const std::string& f_name, double default_value, bool* it_worked)
{
	PDataLeaf leaf(Find(f_name));
	if (!leaf.ok()) {
		if (it_worked) {
			*it_worked = false;
		}
		return default_value;
	}
	std::string s = leaf->GetValue();
	double result = Util::FromIntString(s);
	if (it_worked) {
		*it_worked = true;
	}
	return result;
}
std::string DataList::GetString(const std::string& f_name, const std::string& default_value, bool* it_worked)
{
	PDataLeaf leaf(Find(f_name));
	if (!leaf.ok()) {
		if (it_worked) {
			*it_worked = false;
		}
		return default_value;
	}
	if (it_worked) {
		*it_worked = true;
	}
	return leaf->GetValue();
}

std::string DataLeaf::GetValue()
{
	return value;
}
void DataLeaf::SetValue(const std::string& value_)
{
	value = value_;
}

void DataLeaf::Save(TDFWriter& f)
{
	f.AppendStr(Name(), GetValue());
}
void DataLeaf::Load(Tokenizer& f)
{
	Token t = f.TakeToken();
	value = t.value_s;
	t = f.TakeToken();
	if (t.value_s != ";") {
		f.ReportError(t, "; expected");
	}
}

PDataList ParseTDF(std::istream& s, int* error_count)
{
	Tokenizer t;
	t.EnterStream(s);
	PDataList result(new DataList);
	result->Load(t);
	if (error_count) {
		*error_count = t.NumErrors();
	}
	return result;
}
}
} // namespace LSL { namespace TDF {
