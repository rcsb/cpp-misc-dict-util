// A class for parsing and managing conditional contexts

#include <iostream>
#include <exception>
#include <vector>
#include <time.h>
#include <string.h> // For memset declaration

#include "CifString.h"
#include "ConditionalContext.h"
#include "CifDataInfo.h"

using std::exception;
using std::ifstream;
using std::cout;
using std::cerr;
using std::getline;
using std::endl;


// Internal comparison code from string
enum cmp_code {
  eEqual,
  eNotEqual,
  eLessThan,
  eLessEqualThan,
  eGreaterThan,
  eGreaterEqualThan,
  eUnknown
};

static bool cmpDateTime(const string &op, const string &left, const string &right);
static bool cmpString(const string &op, const string &left, const string &right);
static void parse_date(struct tm& tm, const std::string &val);
static cmp_code getCmpCode(const string &op);
static ConditionalContextItemAction getItemActionEnum(const std::string &action);

static cmp_code getCmpCode(const string &op) {
  // Converts a comparison code to an enumeration for use in switch
  if (op == "=") return eEqual;
  if (op == "!=") return eNotEqual;
  if (op == "<") return eLessThan;
  if (op == "<=") return eLessEqualThan;
  if (op == ">") return eGreaterThan;
  if (op == ">=") return eGreaterEqualThan;  

  return eUnknown;
}


// Constructor
ConditionalContext::ConditionalContext(Block& inBlock, DicFile *dictFileP) :_inBlock(inBlock), cDataInfo(*dictFileP)
{
  // std::cout << "Conditional context init" << std::endl;
  _catConditionalContext = NULL;
  _itemConditionalContext = NULL;
  _dictFileP = dictFileP;

  Block& block = dictFileP->GetBlock(dictFileP->GetFirstBlockName());
  pdbxCatConditionalContext = block.GetTablePtr("pdbx_category_conditional_context");
  pdbxItemConditionalContext = block.GetTablePtr("pdbx_item_conditional_context");
  pdbxConditionalContextList = block.GetTablePtr("pdbx_conditional_context_list");

  CifDataInfo cifDataInfo(*dictFileP);
}


bool ConditionalContext::HaveConditionalTableContext(const string& tableName) {

  // If no conditional context categories
  if (pdbxCatConditionalContext == NULL)
    return false;

  unsigned int queryResult = _getConditionalTableRow(tableName);
  if (queryResult != pdbxCatConditionalContext->GetNumRows()) {
    return true;
  }

  return false;
}

bool ConditionalContext::SkipTable(const string& tableName) {
  // Determines of an entrire category is to be skipped -- return true if so


  if (pdbxCatConditionalContext == NULL)
    return false;

  unsigned int queryResult = _getConditionalTableRow(tableName);

  // If so - test the conditional
  if (queryResult != pdbxCatConditionalContext->GetNumRows()) {

    // Someday iterate possibly
    const string& action = (*pdbxCatConditionalContext)(queryResult, "action");
    const string& contextId = (*pdbxCatConditionalContext)(queryResult, "context_id");

    if (action != "suppress") {
      throw InvalidOptionsException("ConditionalContext::SkipTable unknown action " + action);
    }

    bool ret = _evalConditionalList(contextId, true, tableName);
    if (ret && action == "suppress")
      return true;
  }

  // Else fall through - either no conditional context or allowed
  return false;
}

unsigned int ConditionalContext::_getConditionalTableRow(const string& tableName) {
  // See if table in pdbx_category_conditional_context
  // Returns count to row in category conditional context with category - or GetNumRows()
  vector<string> queryTarget;
  queryTarget.push_back(tableName);

  vector<string> queryCat;
  queryCat.push_back("category_id");

  unsigned int queryResult = pdbxCatConditionalContext->FindFirst(queryTarget,
								  queryCat);
  
  return queryResult;
}


unsigned int ConditionalContext::_getConditionalItemRow(const string& itemName) {
  // Returns row of conditional context if it exists or GetNumRows()

  vector<string> queryTarget;
  queryTarget.push_back(itemName);

  vector<string> queryCat;
  queryCat.push_back("item_name");

  unsigned int queryResult = pdbxItemConditionalContext->FindFirst(queryTarget,
								    queryCat);
  return queryResult;
}


bool ConditionalContext::HaveConditionalItemContext(const string& itemName) {
  // See if item exists in pdbx_item_conditional_context
  // itemname of form _category.item

  if (pdbxItemConditionalContext == NULL)
    return false;
  
  unsigned int queryResult = _getConditionalItemRow(itemName);
  
  if (queryResult != pdbxItemConditionalContext->GetNumRows()) {
    return true;
  }

  return false;
}

bool ConditionalContext::SkipItem(const string& itemName) {
  // Determines of an entrire column is to be skipped -- return true if so


  if (pdbxItemConditionalContext == NULL)
    return false;

  unsigned int queryResult = _getConditionalItemRow(itemName);

  // If so - test the conditional
  if (queryResult != pdbxItemConditionalContext->GetNumRows()) {

    // Someday iterate possibly
    const string& action = (*pdbxItemConditionalContext)(queryResult, "action");
    const string& contextId = (*pdbxItemConditionalContext)(queryResult, "context_id");

    if (action != "suppress-item" && action != "suppress-value" && action != "suppress-row") {
      throw InvalidOptionsException("ConditionalContext::SkipItem unknown action " + action);
    }

    // Suppress-value allows item out to the public.
    if (action == "suppress-value")
      return false;
    

    // Safety checks
    string tableName, colName;
    CifString::GetCategoryFromCifItem(tableName, itemName);
    CifString::GetItemFromCifItem(colName, itemName);
    

    // If category not in file - skip
    if (!_inBlock.IsTablePresent(tableName)) {
      return true;
    }

    // If column not in file - skip - should never happen
    ISTable* tobj = _inBlock.GetTablePtr(tableName);
    if (!tobj->IsColumnPresent(colName)) {
      return true;
    }

    // Iterate rows
    for (unsigned int row = 0; row < tobj->GetNumRows(); row++) {
      bool ret = _evalConditionalList(contextId, false, tableName, colName, row);

      // If action is suppress-item - any true will suppress
      if (ret && action == "suppress-item")
	return true;

      // for suppress-row - if any allowed through - then we allow
      if (ret == false && action == "suppress-row")
	return false;
      }
  }

  // Else fall through - either no conditional context or allowed
  return false;
}

ConditionalContextItemAction ConditionalContext::GetConditionalItemContext(const string& itemName, unsigned int row) {

  if (pdbxItemConditionalContext == NULL)
    return eSuppressNone;

  unsigned int queryResult = _getConditionalItemRow(itemName);

  // If so - test the conditional

  if (queryResult != pdbxItemConditionalContext->GetNumRows()) {

    const string& action = (*pdbxItemConditionalContext)(queryResult, "action");
    const string& contextId = (*pdbxItemConditionalContext)(queryResult, "context_id");

    ConditionalContextItemAction eAction = getItemActionEnum(action);
    if (eAction == eSuppressUnknown) {
      throw InvalidOptionsException("ConditionalContext::GetConditionalItemContext unknown action " + action);
    }

    // Safety checks

    string tableName, colName;
    CifString::GetCategoryFromCifItem(tableName, itemName);
    CifString::GetItemFromCifItem(colName, itemName);
    
    if (!_inBlock.IsTablePresent(tableName)) {
      return eSuppressNone;
    }

    ISTable* tobj = _inBlock.GetTablePtr(tableName);
    if (!tobj->IsColumnPresent(colName)) {
      return eSuppressNone;
    }

    if (row >= tobj->GetNumRows())
      throw out_of_range("Invalid row ConditionalContext::GetConditionalItemContext");

    bool ret = _evalConditionalList(contextId, false, tableName, colName, row);

    if (!ret)
      return eSuppressNone;

    // Suppress...
    return eAction;
  }

  // Else fall through - either no conditional context or allowed
  return eSuppressNone;
}


// Evaluates a conditional context list.
// catContext: If this is a category or item test
// catName, itemName --> specific category/item
// row: row of a category if item conditional list
bool ConditionalContext::_evalConditionalList(const std::string &context_id,
					      bool catContext, const std::string &catName) {
  return _evalConditionalList(context_id, catContext, catName, "", 0);
}


bool ConditionalContext::_evalConditionalList(const std::string &context_id,
					      bool catContext,
					      const std::string &catName, const std::string &item, int row)
{
  // For a given set of conditions identified by context_id, evaluate conditional.
  // Applies to a specific row of a category catname.
  
  // First lookup the specific condition

  // The resulting conditional result
  bool result = false;

  vector<unsigned int> sr;
  vector<string> target_item_names, target_item_values, cmp_ops, log_ops;

  pdbxConditionalContextList->Search(sr, context_id, "context_id");
  if (sr.empty()) {
    throw NotFoundException("Could not find in ConditinalContextList " + context_id);
  }
  // cout << "Number of operators "  << sr.size() << endl;

  target_item_names.clear();
  target_item_values.clear();
  cmp_ops.clear();
  log_ops.clear();

  pdbxConditionalContextList->GetColumn(target_item_names, "target_item_name", sr);
  pdbxConditionalContextList->GetColumn(target_item_values, "target_item_value", sr);
  pdbxConditionalContextList->GetColumn(cmp_ops, "cmp_op", sr);
  pdbxConditionalContextList->GetColumn(log_ops, "log_op", sr);

  for(size_t n = 0 ; n < sr.size(); n++) {
    string target_item_name, target_item_value, cmp_op, log_op;

    target_item_name = target_item_names[n];
    target_item_value = target_item_values[n];
    cmp_op = cmp_ops[n];
    log_op = log_ops[n];

    // cout << "evalConditionalList " << target_item_name << " " << target_item_value << " " << cmp_op << " " << log_op << endl;

    // Optimization if condition is an or - and previous is true, then A or B will be true
    if (!result || (n == 0) || (log_op != "or")) {
      bool ret = _evalConditional(target_item_name, target_item_value, cmp_op, catContext, catName, item, row);
      // cout << "_evalConditional Returned " << ret << endl;

      if ( n == 0 ) {
	result = ret;
      } else {
	if (log_op == "or") {
	  result = result || ret;
	} else if (log_op == "and") {
	  result = result && ret;
	} else
	  throw InvalidOptionsException("logical operater in conditional context " + log_op + "invalid");
      }
    } // optimizaton
    else {
      // cout << "ConditionalContext BYPASS" << endl;
    }		 
  } // for loop


  // std::cout << "evalConditionalList returns " << result << endl;
  return result;
}


bool ConditionalContext::_evalConditional(const std::string &target_item_name, const std::string &target_item_value,
					  const std::string &cmp_op, bool catContext, const std::string &catName, const std::string &item,
					  int rowItem)

{
  // Evaluate a specific operator - some may need to look up row involved for a specific item.
  string lCatName, lItemName;
  ISTable* tobj;

  //cout << "_evalConditional starting " << target_item_name << " " << target_item_value << " " << cmp_op << " " << rowItem<< endl;

  CifString::GetCategoryFromCifItem(lCatName, target_item_name);
  CifString::GetItemFromCifItem(lItemName, target_item_name);  

  // Get the item type
  vector<eTypeCode> eCodes;
  vector<string> attribNames;
  attribNames.push_back(lItemName);
  cDataInfo.GetItemsTypes(eCodes, lCatName, attribNames);

  // Simple operator boolean
  if (cmp_op == "true")
    return true;

  if (cmp_op == "false")
    return false;

  // not_set operator
  if (cmp_op == "not_set") {
    // If category or attribute does not exist - return true
    if (!_inBlock.IsTablePresent(lCatName)) {
      // std::cout << "Conditional category does not exist " << lCatName << std::endl;
      return true;
    }
    tobj = _inBlock.GetTablePtr(lCatName);
    if (!tobj->IsColumnPresent(lItemName)) {
      //std::cout << "Conditional category item does not exist " +  lCatName + "." + lItemName<< std::endl;
      return true;
    }
    // If there are multiple rows - all must pass
    for(unsigned int row = 0; row < tobj->GetNumRows(); row++) {
      const string& val = (*tobj)(row, lItemName);
      if (val != "." && val != "?") {
	//std::cout << "Conditional value set" << endl;
	return false;
      }
    }
    return true;
  }

  // boolean conditional operator
  cmp_code cop = getCmpCode(cmp_op);
  if (cop != eUnknown) {
    // We require category to exist - or raise an error as do not know what to use.  Use "not_set" to block
    if (!_inBlock.IsTablePresent(lCatName)) {
      // throw NotFoundException("Conditional refereences non existant table " + lCatName);
      std::cerr << "ERROR: Conditional category does not exist " << lCatName << std::endl;
      return true;
    }
    tobj = _inBlock.GetTablePtr(lCatName);
    if (!tobj->IsColumnPresent(lItemName)) {
      std::cerr << "ERROR: Conditional category item does not exist " +  lCatName + "." + lItemName<< std::endl;
      return true;
    }

    // std::cout << "Found conditional " << cmp_op << endl;

    // If catContext indicates category - iterate through all rows of item
    vector<int> rows;
    rows.clear();
    if (catContext) {
      for(unsigned int i = 0; i < tobj->GetNumRows(); i++) {
	rows.push_back(i);
      }
    } else {
      rows.push_back(rowItem);
    }

    // Do the work -- for multiple rows - until a conditional returns True -- i.e. or
    bool results = false;
    for(unsigned int i = 0; i < rows.size(); i++){
      int r = rows[i];
      // cout << "Examining row " << r << endl;
      const string& val = (*tobj)(r, lItemName);
      // cout << "Value is " << val << endl;

      bool res = false;
      switch (eCodes[0]) {
      case eTYPE_CODE_DATETIME:
	res = cmpDateTime(cmp_op, val, target_item_value);
	break;

      case eTYPE_CODE_STRING:
	res = cmpString(cmp_op, val, target_item_value);
	break;
	  
      default:
	{
	  cerr << "ERROR: Dictionary requests conditional with type not able to manage " << eCodes[0] << endl;
	  throw NotFoundException("Dictionary type not implemented in conditional ");
	}

      }
      results = results || res;
    }
    return results;
  } // conditional operator

  throw InvalidOptionsException("Unknown operator in _evalConditional " + cmp_op + "\n");

  return false;
}


// Compare time strings
static bool cmpDateTime(const string &op, const string &left, const string &right) {
  //
  string l = left;
  string r = right;
  if (l == "?") l = ".";
  if (r == "?") r = ".";

  if (op == "=") {
    return l == r;
  }
  if (op == "!=") {
    return l != r;
  }

  if (l == "." || r == ".") {
    // Not sure what to do here
    cerr << "Comparison of datetimes and one is missing" << endl;
    return true;
  }

  struct tm ltm, rtm;

  parse_date(ltm, left);
  parse_date(rtm, right);

  time_t lt, rt;
  lt = mktime(&ltm);
  rt = mktime(&rtm);

  // std::cout << "Comparing time " << op << " " << lt << " " << rt << endl;
  cmp_code cop = getCmpCode(op);
  switch (cop) {
  case eEqual:
    return lt == rt;
  case eNotEqual:
    return lt != rt;
  case eLessThan:
    return lt < rt;
  case eLessEqualThan:
    return lt <= rt;
  case eGreaterThan:
    return lt < rt;
  case eGreaterEqualThan:
    return lt <= rt;
  default:
    throw InvalidOptionsException("Invalid datetime comparison " + op);

  }
  // Never reached
  return true;
}

static void parse_date(struct tm& tm, const std::string &val) {
  // Convert time string to structure.  Raises exception if cannot parse
  char *ptr;
  
  memset(&tm, 0, sizeof(tm));

  // Handle two formats.  If more precision in time strings, could extend
  ptr = strptime(val.c_str(), "%Y-%m-%d:%H:%M", &tm);
  if (ptr == NULL)
    ptr = strptime(val.c_str(), "%Y-%m-%d", &tm);

  if (ptr == NULL) {
    throw out_of_range("could not parse date " + val);
  }
  
}

static bool cmpString(const string &op, const string &left, const string &right)
{
  
  if (op == "=")
    return left == right;

  if (op == "!=")
    return left != right;

  // Undefined
  throw InvalidOptionsException("Invalid string comparison " + op);


}

// Convert an item action to Enum
static ConditionalContextItemAction getItemActionEnum(const std::string &action)
{
  if (action == "suppress-row")
    return eSuppressRow;
  if (action == "suppress-value")
    return eSuppressValue;
  if (action == "suppress-item")
    return eSuppressItem;

  return eSuppressUnknown;
}

// Helper to indicate of an item is mandatory in the dictionary
bool ConditionalContext::IsItemMandatory(const string& itemName) {
  return cDataInfo.IsItemMandatory(itemName);
}
