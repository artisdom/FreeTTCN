//
// Copyright (C) 2007 Mateusz Pusz
//
// This file is part of freettcn (Free TTCN) usage example.

// This example is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.

// This example is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this example; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA


#include "cliTestManagement.h"
#include <freettcn/tools/tciValueDumper.h>
extern "C" {
#include <freettcn/ttcn3/tci_value.h>
}
#include <iostream>
#include <sstream>
#include <iomanip>


CCLITestManagement::CCLITestManagement(CMainLoop &mainLoop):
  _mainLoop(mainLoop)
{
}


void CCLITestManagement::ParameterDump(const CModuleParameter &param) const
{
  const unsigned short NAME_WIDTH = 15;
  const unsigned short TYPE_WIDTH = 35;
  
  using namespace std;
  
  ostringstream ostring;
  
  TciValue value = param.DefaultValue();
  TciType type = tciGetType(value);
  
  ostringstream typeString;
  typeString << "<" << (tciGetDefiningModule(type).moduleName ? tciGetDefiningModule(type).moduleName : "{freettcn}") << "." << tciGetName(type) << ">";
  
  ostring.setf(ios::left, ios::adjustfield);
  ostring << setw(TYPE_WIDTH) << typeString.str() << setw(NAME_WIDTH) << param.Name();
  
  if (!tciNotPresent(value)) {
    char buffer[64];
    const char *valueStr = buffer;
    
    TciTypeClassType typeClass = tciGetTypeClass(type);
    switch(typeClass) {
    case TCI_BOOLEAN_TYPE:
      valueStr = freettcn::CTciValueDumper::Boolean2String(value);
      break;
    case TCI_INTEGER_TYPE:
      break;
    default:
      sprintf(buffer, "<Type class: %u not supported>", typeClass);
    }
    
    ostring << " Default: " << valueStr;
  }
  cout << ostring.str();
}


void CCLITestManagement::ModuleInfoPrint() const
{
  const TModuleParList &modParList = ModuleParameterList();
  
  std::cout << "Module Parameters:" << std::endl;
  for(unsigned int i=0; i<modParList.size(); i++) {
    std::cout << " - ";
    ParameterDump(*modParList[i]);
    std::cout << std::endl;
  }
  
  std::cout << std::endl << "Test Cases:" << std::endl;
  TestCasesPrint();
}


void CCLITestManagement::TestCasesPrint() const
{
  const TTCList &tcList = TCList();
  for(TTCList::const_iterator it=tcList.begin(); it != tcList.end(); ++it) {
    TciTestCaseIdType id = (*it)->Id();
    std::cout << " - " << id.moduleName << "." << id.objectName << std::endl;
  }
}


void CCLITestManagement::TestCasesInfoPrint(const std::string &testCaseId) const throw(freettcn::ENotFound)
{
  CTestCase &tc = TestCaseGet(testCaseId);
  
  // obtain test case parameters
  //     TciParameterTypeListType parList = tc.Parameters();
  
  // obtain Test System Interfaces used by the test case 
  TriPortIdList portList = tc.Interface();
  
  std::cout << "Test System Interface:" << std::endl;
  for(int i=0; i<portList.length; i++) {
    std::cout << " - " <<
      portList.portIdList[i]->compInst.compName <<
      " <" << portList.portIdList[i]->compInst.compType.moduleName << "." <<
      portList.portIdList[i]->compInst.compType.objectName << "> " <<
      portList.portIdList[i]->portName << "[" <<
      portList.portIdList[i]->portIndex << "] <" <<
      portList.portIdList[i]->portType.moduleName << "." <<
      portList.portIdList[i]->portType.objectName << ">" <<
      std::endl;
  }
}


void CCLITestManagement::ParametersSet() const
{
  using namespace std;
  
  const TModuleParList &modParList = ModuleParameterList();
  
  for(unsigned int i=0; i<modParList.size(); i++) {
    cout << "Set value for module parameter:" << endl;
    ParameterDump(*modParList[i]);
    cout << endl;

    TciValue defaultValue = modParList[i]->DefaultValue();
    TciType type = tciGetType(defaultValue);
    TciTypeClassType typeClass = tciGetTypeClass(type);
    bool omit = tciNotPresent(defaultValue);
    if (!omit)
      cout << "<ENTER> for DEFAULT" << endl;
    
    const int SIZE = 64; // Buffer size;
    char buf[SIZE];
    bool error;
    do {
      error = false;
      
      string str;
      do {
        cin.getline(buf, SIZE);
        str = buf;
      }
      while(str.empty() && omit);
      
      if (!omit && str.empty())
        break;
      
      switch(typeClass) {
      case TCI_BOOLEAN_TYPE:
        {
          bool value;
          if (str == "TRUE" || str == "true" || str == "1")
            value = true;
          else if (str == "FALSE" || str == "false" || str == "0")
            value = false;
          else
            error = true;
          
          if (!error) {
            TciValue newValue = tciNewInstance(type);
            tciSetBooleanValue(newValue, value);
            modParList[i]->Value(newValue);
          }
        }
        break;
        
      case TCI_INTEGER_TYPE:
        {
          long value;
          
          const char *start = str.c_str();
          char *end;
          value = strtol(start, &end, 0);
          if (end == start || *end != '\0')
            error = true;
          
          if (!error) {
            char buffer[32];
            sprintf(buffer, "%lu", value);
            
            TciValue newValue = tciNewInstance(type);
            tciSetIntAbs(newValue, buffer);
            modParList[i]->Value(newValue);
          }
        }
        break;
        
      default:
        ;
      }
    }
    while(error);
  }
}


void CCLITestManagement::TestCaseStart(const std::string &testCaseId, const TciParameterListType &parameterlist) throw(freettcn::ENotFound)
{
  freettcn::TM::CTestManagement::TestCaseStart(testCaseId, parameterlist);
  _mainLoop.Start();
}


void CCLITestManagement::TestCaseTerminated(TciVerdictValue verdict, const TciParameterListType &parameterlist)
{
  TStatus status = Status();
  freettcn::TM::CTestManagement::TestCaseTerminated(verdict, parameterlist);
  if (status == RUNNING_TEST_CASE)
    _mainLoop.Stop();
}


void CCLITestManagement::TestCaseStop() throw(freettcn::EOperationFailed)
{
  TStatus status = Status();
  freettcn::TM::CTestManagement::TestCaseStop();
  if (status == RUNNING_TEST_CASE)
    _mainLoop.Stop();
}




void CCLITestManagement::ControlStart()
{
  freettcn::TM::CTestManagement::ControlStart();
  _mainLoop.Start();
}


void CCLITestManagement::ControlStop() throw(freettcn::EOperationFailed)
{
  freettcn::TM::CTestManagement::ControlStop();
  _mainLoop.Stop();
}


void CCLITestManagement::ControlTerminated()
{
  freettcn::TM::CTestManagement::ControlTerminated();
  _mainLoop.Stop();
}





CCLITestManagement::CMainLoop::~CMainLoop()
{
}
