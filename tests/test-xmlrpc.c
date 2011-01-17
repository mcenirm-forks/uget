#ifdef _WIN32
#include <winsock.h>
#else
#include <unistd.h>
#endif

#include <UgXmlrpc.h>

void ug_xmlrpc_value_dump (UgXmlrpcValue* value);
void ug_xmlrpc_data_dump  (UgXmlrpcData* xrdata);

void ug_xmlrpc_data_dump (UgXmlrpcData* xrdata)
{
	UgXmlrpcValue*	value;
	guint			index;

	for (index = 0;  index < xrdata->len;  index++) {
		value = xrdata->data + index;
		if (value->name)
			g_print ("name = %s, ", value->name);
		else
			g_print ("index = %d, ", index);
		ug_xmlrpc_value_dump (value);
	}
}

void ug_xmlrpc_value_dump (UgXmlrpcValue* value)
{
	UgXmlrpcData*	xrdata;

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
		g_print ("double = '%f'\n", value->c.double_);
		break;

	case UG_XMLRPC_DATETIME:
		g_print ("datetime = '%s'\n", value->c.datetime);
		break;

	case UG_XMLRPC_BASE64:
		g_print ("base64 = '%s'\n", value->c.base64);
		break;

	case UG_XMLRPC_NIL:
		g_print ("nil\n");
		break;

	case UG_XMLRPC_STRUCT:
		g_print ("struct --- start ---\n");
		xrdata = value->c.struct_;
		ug_xmlrpc_data_dump (xrdata);
		g_print ("struct --- end ---\n");
		break;

	case UG_XMLRPC_ARRAY:
		g_print ("array --- start ---\n");
		xrdata = value->c.struct_;
		ug_xmlrpc_data_dump (xrdata);
		g_print ("array --- end ---\n");
		break;

	default:
		break;
	}
}

void test_xmlrpc (void)
{
	UgXmlrpc		xmlrpc;
	UgXmlrpcData*	xrdata;
	UgXmlrpcValue*	value;

	ug_xmlrpc_init (&xmlrpc);
	ug_xmlrpc_use_client (&xmlrpc, "http://localhost:6800/rpc", NULL);

	xrdata = ug_xmlrpc_data_new (8);
	value  = ug_xmlrpc_data_alloc (xrdata);
	value->type = UG_XMLRPC_STRING;
	value->c.string = "http://aria2.sourceforge.net/aria2c.1.html";

	ug_xmlrpc_call (&xmlrpc, "aria2.addUri",
			UG_XMLRPC_ARRAY, xrdata,
			UG_XMLRPC_NONE);
	if (ug_xmlrpc_response (&xmlrpc) == UG_XMLRPC_RESPONSE_OK) {
		g_print ("aria2.addUri\n");
		ug_xmlrpc_get_param (&xmlrpc, value);
		ug_xmlrpc_value_dump (value);
	}

	g_print ("------\n");

	ug_xmlrpc_call (&xmlrpc, "aria2.tellStatus",
			UG_XMLRPC_STRING, "1",
			UG_XMLRPC_NONE);
	if (ug_xmlrpc_response (&xmlrpc) == UG_XMLRPC_RESPONSE_OK) {
		g_print ("aria2.tellStatus\n");
		ug_xmlrpc_get_param (&xmlrpc, value);
		ug_xmlrpc_value_dump (value);
	}

	ug_xmlrpc_finalize (&xmlrpc);
}

int main (int argc, char* argv[])
{
#ifdef _WIN32
	WSADATA WSAData;

	WSAStartup (MAKEWORD (2, 2), &WSAData);
#endif

	test_xmlrpc ();

#ifdef _WIN32
	WSACleanup ();
#endif

	return 0;	// EXIT_SUCCESS
}
