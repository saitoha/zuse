/* ***** BEGIN LICENSE BLOCK Version: GPL 3.0 ***** 
 * Copyright (C) 2008-2011  Hayaki Saito <saitoha@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 * ***** END LICENSE BLOCK ***** */



#include "../../src/types.hpp"

using namespace ecmascript;
using namespace ecmascript::regular_expression;

int main(int argc, char *argv[])
{
    es_regexp<std::wstring>& regexp = *new es_regexp<std::wstring>(L"(.ab.)", L"g");
    //regexp.internal_print();
    wprintf(regexp.exec(*new es_string<std::wstring>(L"dffabckjabc")).ToString().operator const_string_t const().c_str());
    
    return 0;
}

