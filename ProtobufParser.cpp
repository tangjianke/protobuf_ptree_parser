#include "ProtobufParser.h"
#include <boost/property_tree/detail/ptree_utils.hpp>
#include <google/protobuf/descriptor.h>
#include <vector>
#include <string>

namespace protobuf_parser
{
	typedef std::vector<const FieldDescriptor*> FieldsList;

	void read_protobuf_internal(const Message& message, ptree& node)
	{
		const Reflection*   refl = message.GetReflection();
		FieldsList			fields;
		refl->ListFields(message, &fields);

		FieldsList::iterator it = fields.begin();
		for (; it != fields.end(); it ++)
		{
			const FieldDescriptor* field = *it;

			if (!field->is_repeated())
			{
				stringstream ss;
				std::string value_s;

				switch(field->cpp_type())
				{
				case FieldDescriptor::CPPTYPE_INT32:
					{
						ss << refl->GetInt32(message, field);
						ss >> value_s;
						node.push_back(std::make_pair(field->name(), ptree(value_s)));
					}
					break;
				case FieldDescriptor::CPPTYPE_INT64:
					{
						ss << refl->GetInt64(message, field);
						ss >> value_s;
						node.push_back(std::make_pair(field->name(), ptree(value_s)));
					}
					break;
				case FieldDescriptor::CPPTYPE_UINT32:
					{
						ss << refl->GetUInt32(message, field);
						ss >> value_s;
						node.push_back(std::make_pair(field->name(), ptree(value_s)));
					}
					break;
				case FieldDescriptor::CPPTYPE_DOUBLE:
					{
						ss << refl->GetDouble(message, field);
						ss >> value_s;
						node.push_back(std::make_pair(field->name(), ptree(value_s)));					
					}
					break;
				case FieldDescriptor::CPPTYPE_FLOAT:
					{
						ss << refl->GetFloat(message, field);
						ss >> value_s;
						node.push_back(std::make_pair(field->name(), ptree(value_s)));
					}
					break;
				case FieldDescriptor::CPPTYPE_BOOL:
					{
						ss << (refl->GetBool(message, field) ? "true" : "false" );
						ss >> value_s;
						node.push_back(std::make_pair(field->name(), ptree(value_s)));
					}
					break;
				case FieldDescriptor::CPPTYPE_ENUM:
					{
						ss << refl->GetEnum(message, field)->number();
						ss >> value_s;
						node.push_back(std::make_pair(field->name(), ptree(value_s)));				
					}
					break;
				case FieldDescriptor::CPPTYPE_STRING:
					{
						value_s = refl->GetString(message, field);
						node.push_back(std::make_pair(field->name(), ptree(value_s)));
					}
					break;
				case FieldDescriptor::CPPTYPE_MESSAGE:
					{
						ptree *parent = &node;
						ptree *child = &parent->push_back(std::make_pair(field->name(), ptree()))->second;
						read_protobuf_internal(refl->GetMessage(message, field), *child);
					}
					break;
				default:
					break;
				}
			}
			else //repeated
			{
				ptree *parent = &node;
				ptree &child = parent->push_back(std::make_pair(field->name(), ptree()))->second;

				const int count = refl->FieldSize(message, field);
				for (int i = 0; i < count ; i ++)
				{
					stringstream ss;
					std::string value_s;

					switch(field->cpp_type())
					{
					case FieldDescriptor::CPPTYPE_INT32:
						{
							ss << refl->GetRepeatedInt32(message, field, i);
							ss >> value_s;
							child.push_back(std::make_pair("", ptree(value_s)));
						}
						break;
					case FieldDescriptor::CPPTYPE_INT64:
						{
							ss << refl->GetRepeatedInt64(message, field, i);
							ss >> value_s;
							child.push_back(std::make_pair("", ptree(value_s)));
						}
						break;
					case FieldDescriptor::CPPTYPE_UINT32:
						{
							ss << refl->GetRepeatedUInt32(message, field, i);
							ss >> value_s;
							child.push_back(std::make_pair("", ptree(value_s)));
						}
						break;
					case FieldDescriptor::CPPTYPE_UINT64:
						{
							ss << refl->GetRepeatedUInt64(message, field, i);
							ss >> value_s;
							child.push_back(std::make_pair("", ptree(value_s)));
						}
						break;
					case FieldDescriptor::CPPTYPE_DOUBLE:
						{
							ss << refl->GetRepeatedDouble(message, field, i);
							ss >> value_s;
							child.push_back(std::make_pair("", ptree(value_s)));
						}
						break;
					case FieldDescriptor::CPPTYPE_FLOAT:
						{
							ss << refl->GetRepeatedFloat(message, field, i);
							ss >> value_s;
							child.push_back(std::make_pair("", ptree(value_s)));
						}
						break;
					case FieldDescriptor::CPPTYPE_BOOL:
						{
							ss << (refl->GetRepeatedBool(message, field, i) ? "true" : "false");
							ss >> value_s;
							child.push_back(std::make_pair("", ptree(value_s)));
						}
						break;
					case FieldDescriptor::CPPTYPE_ENUM:
						{
							ss << refl->GetRepeatedEnum(message, field, i)->number();
							ss >> value_s;
							child.push_back(std::make_pair("", ptree(value_s)));
						}
						break;
					case FieldDescriptor::CPPTYPE_STRING:
						{
							value_s = refl->GetRepeatedString(message, field, i);
							child.push_back(std::make_pair("", ptree(value_s)));
						}
						break;
					case FieldDescriptor::CPPTYPE_MESSAGE:
						{
							ptree *parent = &child;
							ptree *grandson = &parent->push_back(std::make_pair("", ptree()))->second;
							read_protobuf_internal(refl->GetRepeatedMessage(message, field, i), *grandson);
						}
						break;
					default:
						break;
					}

				}
			}
		}
	}

	void write_protobuf_internal(Message& message, const ptree &node)
	{
		const Reflection*	refl = message.GetReflection();
		const Descriptor*	desc = message.GetDescriptor();
		
		for (int i = 0; i < desc->field_count(); i++) 
		{
			const FieldDescriptor* field = desc->field(i);
			if (!field->is_repeated())
			{
				ptree::const_assoc_iterator asit = node.find(field->name()) ;
				if (asit == node.not_found())
				{
					continue;
				}
				const ptree& child = asit->second;

				stringstream ss;
				switch(field->cpp_type())
				{
				case FieldDescriptor::CPPTYPE_INT32:
					{
						ss << child.get_value<std::string>();
						int32 value = 0;
						ss >> value ;
						refl->SetInt32(&message, field, value);
					}
					break;
				case FieldDescriptor::CPPTYPE_INT64:
					{
						ss << child.get_value<std::string>();
						int64 value = 0;
						ss >> value ;
						refl->SetInt64(&message, field, value);						
					}
					break;
				case FieldDescriptor::CPPTYPE_DOUBLE:
					{
						ss << child.get_value<std::string>();
						double value = 0;
						ss >> value ;
						refl->SetDouble(&message, field, value);					
					}
					break;
				case FieldDescriptor::CPPTYPE_FLOAT:
					{
						ss << child.get_value<std::string>();
						float value = 0;
						ss >> value ;
						refl->SetFloat(&message, field, value);					
					}
					break;
				case FieldDescriptor::CPPTYPE_BOOL:
					{
						ss << ( child.get_value<std::string>() == "true" ? true : false );
						bool value = false;
						ss >> value ;
						refl->SetBool(&message, field, value);					
					}
					break;
				case FieldDescriptor::CPPTYPE_ENUM:
					{ 
						ss << child.get_value<std::string>();
						int value = 0;
						ss >> value ;
						refl->SetEnum(&message, field, field->enum_type()->FindValueByNumber(value));	
					}
					break;
				case FieldDescriptor::CPPTYPE_STRING:
					{
						std::string value = child.get_value<std::string>();
						refl->SetString(&message, field, value);
					}
					break;
				case FieldDescriptor::CPPTYPE_MESSAGE:
					{
						write_protobuf_internal(*refl->MutableMessage(&message, field), child);
					}
					break;
				default:
					break;
				}
			}
			else //repeated
			{
				ptree::const_assoc_iterator asit = node.find(field->name()) ;
				if (asit == node.not_found())
				{
					continue;
				}
				const ptree& child = asit->second;
				if(child.count("") != child.size())
				{
					continue;
				}
				ptree::const_iterator childIt = child.begin();
				for (; childIt != child.end(); ++ childIt)
				{
					stringstream ss;
					const ptree &pt = childIt->second;
					switch(field->cpp_type())
					{
					case FieldDescriptor::CPPTYPE_INT32:
						{
							ss << pt.get_value<std::string>();
							int32 value = 0;
							ss >> value ;
							refl->AddInt32(&message, field, value);
						}
						break;
					case FieldDescriptor::CPPTYPE_INT64:
						{
							ss << pt.get_value<std::string>();
							int64 value = 0;
							ss >> value ;
							refl->AddInt64(&message, field, value);
						}
						break;
					case FieldDescriptor::CPPTYPE_UINT32:
						{
							ss << pt.get_value<std::string>();
							uint32 value = 0;
							ss >> value ;
							refl->AddUInt32(&message, field, value);
						}
						break;
					case FieldDescriptor::CPPTYPE_UINT64:
						{
							ss << pt.get_value<std::string>();
							int64 value = 0;
							ss >> value ;
							refl->AddUInt64(&message, field, value);
						}
						break;
					case FieldDescriptor::CPPTYPE_DOUBLE:
						{
							ss << pt.get_value<std::string>();
							double value = 0;
							ss >> value ;
							refl->AddDouble(&message, field, value);
						}
						break;
					case FieldDescriptor::CPPTYPE_FLOAT:
						{
							ss << pt.get_value<std::string>();
							float value = 0;
							ss >> value ;
							refl->AddFloat(&message, field, value);
						}
						break;
					case FieldDescriptor::CPPTYPE_BOOL:
						{
							ss << ( child.get_value<std::string>() == "true" ? true : false );
							bool value = false;
							ss >> value ;
							refl->AddBool(&message, field, value);
						}
						break;
					case FieldDescriptor::CPPTYPE_ENUM:
						{
							ss << pt.get_value<std::string>();
							int value = false;
							ss >> value ;
							refl->AddEnum(&message, field, field->enum_type()->FindValueByNumber(value));
						}
						break;
					case FieldDescriptor::CPPTYPE_STRING:
						{
							std::string value = pt.get_value<std::string>();
							refl->AddString(&message, field, value);
						}
						break;
					case FieldDescriptor::CPPTYPE_MESSAGE:
						{	
							write_protobuf_internal(*refl->AddMessage(&message, field), pt);
						}
						break;
					}
				}
			}
		}
		
	}

	void read_protobuf( const Message& message, ptree &pt )
	{
		pt.clear();
		read_protobuf_internal(message, pt);
	}

	void write_protobuf( Message& message, const ptree &pt )
	{
		message.Clear();
		write_protobuf_internal(message, pt);
	}

}
