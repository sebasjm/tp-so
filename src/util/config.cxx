#include "config.h"

#include <iostream>
#include <libxml/parser.h>
#include <libxml/xmlstring.h>
#include <string.h>

using namespace std;

char* read_config(const char* config, const char* prop) {
    xmlDocPtr doc = xmlReadFile(config,NULL,XML_PARSE_NOERROR | XML_PARSE_NOWARNING | XML_PARSE_NONET  );
    if (!doc) return NULL;
    xmlNodePtr cur = xmlDocGetRootElement(doc);
    if (!cur) return NULL;
    for (xmlAttrPtr i = cur->properties; i != NULL; i = i->next ) {
        if ( !strcmp((char*)i->name ,prop) ) {
            int len = strlen((char*)i->children->content);
            char* result = (char*)malloc( len+1);
            memcpy(result,(char*)i->children->content,len+1);
            return result;
        }
    }
    return NULL;
}
