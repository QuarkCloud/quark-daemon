
#include "qkhttp/Header.h"

namespace qkhttp {

Field::Field()
{
    //
}
Field::Field(const Field& f):key(f.key) , value(f.value)
{
    //
}
Field::Field(const std::string& k, const std::string& v):key(k) , value(v)
{
    //
}
Field::~Field()
{
    //
}
bool Field::valid() const
{
    return (key.empty() == false && value.empty() == false);
}

const Field Field::kEmpty;
const std::string Field::kEmptyValue;

HeaderFields::HeaderFields()
{
    //
}
HeaderFields::~HeaderFields()
{
    //
}
bool HeaderFields::push(const std::string& key, const std::string& value)
{
    int index = (int)fields_.size();
    fields_.push_back(Field(key, value));
    return indexes_.insert(std::pair<std::string, int>(key, index)).second;
}
const std::string& HeaderFields::find(const std::string& key) const
{
    std::map<std::string, int>::const_iterator citer = indexes_.find(key);
    if (citer == indexes_.end())
        return Field::kEmptyValue;
    return get(citer->second).value;
}
int HeaderFields::size() const
{
    return (int)fields_.size();
}
const Field& HeaderFields::get(int index) const
{
    if (index < 0 || index >= (int)fields_.size())
        return Field::kEmpty;
    return fields_[index];
}

Request::Request()
{
    //
}
Request::~Request()
{
    //
}

 
Response::Response()
{
    //
}
Response::~Response()
{
    //
}

}
