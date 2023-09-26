#ifndef HTMLPARSER_H
#define HTMLPARSER_H
#include <QString>
#include <iostream>

#include <tidy.h>
#include <tidybuffio.h>
#include <stdio.h>
#include <errno.h>
#include <sstream>
#include <pugixml/pugixml.hpp>



class CSoup
{
private:
    pugi::xml_document doc;
public:
    CSoup(std::string html) {
        parse(tidy(html.data()).data());
    }

    ~CSoup(){}

    inline const char* selectText(std::string XPath) const {
        return doc.select_node(XPath.data()).node().child_value ();
    }
    inline pugi::xpath_node_set select(std::string XPath) const {
        return doc.select_nodes(XPath.data());
    }
    inline pugi::xpath_node selectFirst(const char* XPath) const {
        return doc.select_node(XPath);
    }

private:
    std::string tidy(const char* input);

    bool parse (const char* source);
};

#endif // HTMLPARSER_H
