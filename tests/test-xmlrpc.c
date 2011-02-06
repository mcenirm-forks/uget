#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef _WIN32
#include <winsock.h>
#endif

#ifdef HAVE_PLUGIN_ARIA2

#include <UgXmlrpc.h>

void ug_xmlrpc_value_dump (UgXmlrpcValue* value);

void ug_xmlrpc_value_dump_data (UgXmlrpcValue* value)
{
	UgXmlrpcValue*	child;
	guint			index;

	for (index = 0;  index < value->len;  index++) {
		child = value->data + index;
		if (child->name)
			g_print ("name = %s, ", child->name);
		else
			g_print ("index = %d, ", index);
		ug_xmlrpc_value_dump (child);
	}
}

void ug_xmlrpc_value_dump (UgXmlrpcValue* value)
{
	switch (value->type) {
	case UG_XMLRPC_INT:
		g_print ("int = %d\n", value->c.int_);
		break;

	case UG_XMLRPC_INT64:
		g_print ("int64 = %lld\n", value->c.int64);
		break;

	case UG_XMLRPC_BOOLEAN:
		g_print ("boolean = %d\n", value->c.int_);
		break;

	case UG_XMLRPC_STRING:
		g_print ("string = '%s'\n", value->c.string);
		break;

	case UG_XMLRPC_DOUBLE:
		g_print ("double = %f\n", value->c.double_);
		break;

	case UG_XMLRPC_DATETIME:
		g_print ("datetime = '%s'\n", value->c.datetime);
		break;

	case UG_XMLRPC_BINARY:
		g_print ("binary data, length = %u\n", value->len);
		break;

	case UG_XMLRPC_NIL:
		g_print ("nil\n");
		break;

	case UG_XMLRPC_ARRAY:
		g_print ("array --- start ---\n");
		ug_xmlrpc_value_dump_data (value);
		g_print ("array --- end ---\n");
		break;

	case UG_XMLRPC_STRUCT:
		g_print ("struct --- start ---\n");
		ug_xmlrpc_value_dump_data (value);
		g_print ("struct --- end ---\n");
		break;

	default:
		break;
	}
}

void test_xmlrpc (void)
{
	UgXmlrpc			xmlrpc;
	UgXmlrpcValue*		value;
	UgXmlrpcResponse	response;

	ug_xmlrpc_init (&xmlrpc);
	ug_xmlrpc_use_client (&xmlrpc, "http://localhost:6800/rpc", NULL);

	g_print ("aria2.getVersion\n");
	response = ug_xmlrpc_call (&xmlrpc, "aria2.getVersion", UG_XMLRPC_NONE);
	if (response != UG_XMLRPC_ERROR) {
		value = ug_xmlrpc_get_value (&xmlrpc);
		ug_xmlrpc_value_dump (value);
	}

	g_print ("------\n");

	g_print ("aria2.tellStatus\n");
	response = ug_xmlrpc_call (&xmlrpc, "aria2.tellStatus",
			UG_XMLRPC_STRING, "1",
			UG_XMLRPC_NONE);
	if (response != UG_XMLRPC_ERROR) {
		value = ug_xmlrpc_get_value (&xmlrpc);
		ug_xmlrpc_value_dump (value);
	}

	ug_xmlrpc_finalize (&xmlrpc);
}

#endif	// HAVE_PLUGIN_ARIA2

int main (int argc, char* argv[])
{
#ifdef _WIN32
	WSADATA WSAData;

	WSAStartup (MAKEWORD (2, 2), &WSAData);
#endif

#ifdef HAVE_PLUGIN_ARIA2
	test_xmlrpc ();
#endif

#ifdef _WIN32
	WSACleanup ();
#endif

	return 0;	// EXIT_SUCCESS
}

