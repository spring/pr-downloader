/* This file is part of the Springlobby (GPL v2 or later), see COPYING */

#ifndef LSL_HEADERGUARD_TDFCONTAINER_H
#define LSL_HEADERGUARD_TDFCONTAINER_H

#include <lslutils/misc.h>
#include <lslutils/conversion.h>
#include <lslutils/type_forwards.h>
#include <lslutils/autopointers.h>

#include <sstream>
#include <vector>
#include <deque>
#include <map>

namespace LSL
{
namespace TDF
{

/** \brief std::stringstream based output class for TDF
 * this is only ever used internally (script generation)
 * and needn't be exposed to library users
 * \todo add link to format specification
 **/
class TDFWriter
{
public:
	TDFWriter(std::stringstream& s);
	~TDFWriter();
	void EnterSection(const std::string& name);
	void LeaveSection();
	void Indent();
	std::string GetCurrentPath();
	void AppendStr(const std::string& name, std::string value);
	void AppendInt(const std::string& name, int value);
	void AppendFloat(const std::string& name, float value);
	/*
		/// works like algorithms such as std::sort
		template<class T> void Append( const std::string& name, T begin, T end );
*/
	void AppendLineBreak();
	void Close();

private:
	std::stringstream& m_stream;
	int m_depth;
};

///
/// Parsing classes contributed by dmytry aka dizekat, copypasted from personal
/// project: render_control_new/utils/parsing.h and cpp , database.h and cpp
///

class Tokenizer;
class Node;
typedef RefcountedPointer<Node> PNode;
class DataList;
typedef RefcountedPointer<DataList> PDataList;
class DataLeaf;
typedef RefcountedPointer<DataLeaf> PDataLeaf;

class Node : public RefcountedContainer, public boost::noncopyable
{
	friend class DataList;

private:
	DataList* parent;
	Node* list_prev;
	Node* list_next;
	void ListRemove();
	void ListInsertAfter(Node* what);
	std::string name;

public:
	std::string Name();

	// Sets the name, and updates parent if present
	bool SetName(const std::string& name_);
	Node()
	    : parent(NULL)
	    , list_prev(NULL)
	    , list_next(NULL)
	    , name(std::string())
	{
	}
	virtual ~Node();
	DataList* Parent() const; // parent list
	//void SetParent(DataList *parent_);
	bool IsChildOf(DataList* what) const;

	virtual void Save(TDFWriter& f);
	virtual void Load(Tokenizer& f);
};


/** \brief TDF Parsing
 * Usage
 * Parsing:

 * int errs=0;
 * PDataList root(ParseTDF(some istream, &errs));// errs is optional, gives number of errors when loading.

 * Getting values:

 * PDataList game(root->Find(\1
 * if(!game.ok()){slLogMessage(\1;return false;}
 * std::string gamename=game->GetString(\1;

 * (see optional parameters for setting default and knowing if it failed)
 * and so on and so forth.
 * Saving tdf:
**/
class DataList : public Node
{
private:
	std::map<std::string, PNode> nodes;
	Node list_loop; // next is first, prev is last in the list
	typedef std::map<std::string, PNode>::iterator nodes_iterator;

public:
	DataList();
	~DataList();

	bool Insert(PNode node);		// return false if such entry already exists.
	bool InsertAt(PNode node, PNode where); // return false if such entry already exists. Inserts right before given

	// rename if such entry already exists. Names is like name!1 or name!2 etc
	void InsertRename(PNode node);
	void InsertRenameAt(PNode node, PNode where);
	bool Remove(const std::string& str);
	bool Remove(PNode node);
	bool Rename(const std::string& old_name, const std::string& new_name);
	PNode Find(const std::string& str); // find by name

	std::string Path();

	PNode FindByPath(const std::string& str);

	PNode Next(PNode what);
	PNode Prev(PNode what);
	PNode End();
	PNode First();
	PNode Last();

	virtual void Save(TDFWriter& f);
	virtual void Load(Tokenizer& f);

	int GetInt(const std::string& name, int default_value = 0, bool* it_worked = NULL);
	double GetDouble(const std::string& name, double default_value = 0, bool* it_worked = NULL);
	std::string GetString(const std::string& name, const std::string& default_value = std::string(), bool* it_worked = NULL);
};

//! docme
class DataLeaf : public Node
{
	friend class DataList;

private:
	std::string value;

public:
	std::string GetValue();
	void SetValue(const std::string& value);

	virtual void Save(TDFWriter& f);
	virtual void Load(Tokenizer& f);
};

struct Token
{
	enum TokenType {
		type_none,
		type_section_name,
		type_enter_section,
		type_leave_section,
		type_entry_name,
		type_entry_value,
		type_semicolon,
		type_eof
	};
	TokenType type;
	std::string value_s;

	std::string pos_string; // for error reporting

	bool IsEOF() const
	{
		return (type == type_eof);
	}
	Token()
	    : type(type_eof)
	{
	}
};

PDataList ParseTDF(std::istream& s, int* error_count = NULL);
}
} // namespace LSL { namespace TDF {

#endif // LSL_HEADERGUARD_TDFCONTAINER_H
