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
		g_print ("int64 = %lld\n", (long long) value->c.int64);
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

void aria2_getVersion (UgXmlrpc* xmlrpc)
{
	UgXmlrpcValue*		value;
	UgXmlrpcResponse	response;

	g_print ("aria2.getVersion\n");
	response = ug_xmlrpc_call (xmlrpc,
			"aria2.getVersion",
			UG_XMLRPC_NONE);
	switch (response) {
	case UG_XMLRPC_ERROR:
		g_print ("- error\n");
		break;

	case UG_XMLRPC_FAULT:
	case UG_XMLRPC_OK:
		value = ug_xmlrpc_get_value (xmlrpc);
		ug_xmlrpc_value_dump (value);
		break;
	}
	g_print ("\n");
}

void aria2_tellStatus (UgXmlrpc* xmlrpc)
{
	UgXmlrpcValue*		value;
	UgXmlrpcResponse	response;

	g_print ("aria2.tellStatus\n");
	response = ug_xmlrpc_call (xmlrpc,
			"aria2.tellStatus",
			UG_XMLRPC_STRING, "1",
			UG_XMLRPC_NONE);
	switch (response) {
	case UG_XMLRPC_ERROR:
		g_print ("- error\n");
		break;

	case UG_XMLRPC_FAULT:
	case UG_XMLRPC_OK:
		value = ug_xmlrpc_get_value (xmlrpc);
		ug_xmlrpc_value_dump (value);
		break;
	}
	g_print ("\n");
}

void aria2_changeGlobalOption (UgXmlrpc* xmlrpc)
{
	UgXmlrpcValue*		options;
	UgXmlrpcValue*		member;
	UgXmlrpcValue*		value;
	UgXmlrpcResponse	response;

	g_print ("aria2.changeGlobalOption\n");
	options = ug_xmlrpc_value_new_struct (2);
	// max-overall-download-limit
	member = ug_xmlrpc_value_alloc (options);
	member->name = "max-overall-download-limit";
	member->type = UG_XMLRPC_STRING;
	member->c.string = g_strdup_printf ("%dK", 0);
	// max-overall-upload-limit
	member = ug_xmlrpc_value_alloc (options);
	member->name = "max-overall-upload-limit";
	member->type = UG_XMLRPC_STRING;
	member->c.string = g_strdup_printf ("%dK", 0);

	response = ug_xmlrpc_call (xmlrpc,
			"aria2.changeGlobalOption",
			UG_XMLRPC_STRUCT, options,
			UG_XMLRPC_NONE);
	switch (response) {
	case UG_XMLRPC_ERROR:
		g_print ("- error\n");
		break;

	case UG_XMLRPC_FAULT:
	case UG_XMLRPC_OK:
		value = ug_xmlrpc_get_value (xmlrpc);
		ug_xmlrpc_value_dump (value);
		break;
	}
	g_print ("\n");

	ug_xmlrpc_value_free (options);
}

void test_xmlrpc (void)
{
	UgXmlrpc*			xmlrpc;

	xmlrpc = g_malloc0 (sizeof (UgXmlrpc));
	ug_xmlrpc_init (xmlrpc);
	ug_xmlrpc_use_client (xmlrpc, "http://localhost:6800/rpc", NULL);

	aria2_getVersion (xmlrpc);
	aria2_tellStatus (xmlrpc);
	aria2_changeGlobalOption (xmlrpc);

	ug_xmlrpc_finalize (xmlrpc);
	g_free (xmlrpc);
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

