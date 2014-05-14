#pragma once
#include <boost/property_tree/ptree.hpp>
#include <google/protobuf/message.h>

namespace protobuf_parser
{
	using namespace boost::property_tree;
	using namespace google::protobuf;

	void read_protobuf(const Message& message, ptree &pt);
	void write_protobuf(Message& message, const ptree &pt);
}
