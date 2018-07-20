/*
 ISC License

 Copyright (c) 2016-2017, Autonomous Vehicle Systems Lab, University of Colorado at Boulder

 Permission to use, copy, modify, and/or distribute this software for any
 purpose with or without fee is hereby granted, provided that the above
 copyright notice and this permission notice appear in all copies.

 THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

 */
//
//  jsonParser.hpp
//  Visualization
//
//  Created by Patrick Kenneally on 8/9/16.
//
//

#ifndef jsonParser_hpp
#define jsonParser_hpp

#include <stdio.h>
#include <iostream>
#include <boost/property_tree/ptree.hpp>

class JsonParser {
    
public:
    JsonParser();
    ~JsonParser();
    std::string parseToJson();
    void parseFromJson();
    
private:
    boost::property_tree::ptree root;
};

#endif /* jsonParser_hpp */
