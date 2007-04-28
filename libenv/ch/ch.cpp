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

/**
 * @file   ch.cpp
 * @author Mateusz Pusz
 * @date   Fri Apr  6 17:55:10 2007
 * 
 * @brief  
 * 
 * 
 */

#include "ch.h"
extern "C" {
#include "freettcn/tci_ch_te.h"
}

#include <iostream>



freettcn::CH::CLogMask::CLogMask(bool enabled /* true */):
  freettcn::CLogMask(freettcn::LOG_CH_NUM, enabled)
{
}

freettcn::CH::CLogMask::~CLogMask()
{
}


freettcn::CH::CComponentHandler *freettcn::CH::CComponentHandler::_instance = 0;

freettcn::CH::CComponentHandler &freettcn::CH::CComponentHandler::Instance() throw(ENotFound)
{
  if (_instance)
    return *_instance;
  
  throw ENotFound();
}

freettcn::CH::CComponentHandler::CComponentHandler()
{
  _instance = this;
}

freettcn::CH::CComponentHandler::~CComponentHandler()
{
  //  Clear();
  _instance = 0;
}


void freettcn::CH::CComponentHandler::ResetReq()
{
  const unsigned int teNum = 1;                     /**< @todo define TEs for the test case (having system ports of the indicated TC) */
  //execute test case on all TEs
  for (unsigned int i = 0; i < teNum; i++)
    /// @todo for each TE
    tciReset();
}


TriComponentId freettcn::CH::CComponentHandler::TestComponentCreateReq(TciTestComponentKindType kind, TciType componentType, String name)
{
  return TestComponentCreate(kind, componentType, name);
}

TriComponentId freettcn::CH::CComponentHandler::TestComponentCreate(TciTestComponentKindType kind, TciType componentType, String name)
{
  return tciCreateTestComponent(kind, componentType, name);
}




void freettcn::CH::CComponentHandler::TestComponentStartReq(const TriComponentId &component, const TciBehaviourIdType &behavior, const TciParameterListType &parameterList)
{
  TestComponentStart(component, behavior, parameterList);
}


void freettcn::CH::CComponentHandler::TestComponentStart(const TriComponentId &component, const TciBehaviourIdType &behavior, const TciParameterListType &parameterList)
{
  return tciStartTestComponent(component, behavior, parameterList);
}



void freettcn::CH::CComponentHandler::TestComponentTerminatedReq(const TriComponentId &component, TciVerdictValue verdict)
{
  const unsigned int teNum = 1;                     /**< @todo define TEs for the test case */
  for (unsigned int i = 0; i < teNum; i++) {
    /// @todo for each TE
    TestComponentTerminated(component, verdict);
  }
}

void freettcn::CH::CComponentHandler::TestComponentTerminated(const TriComponentId &component, TciVerdictValue verdict)
{
  tciTestComponentTerminated(component, verdict);
}



void freettcn::CH::CComponentHandler::TestCaseExecuteReq(const TciTestCaseIdType &testCaseId, const TriPortIdList &tsiPortList)
{
  const unsigned int teNum = 1;                     /**< @todo define TEs for the test case (having system ports of the indicated TC) */
  //execute test case on all TEs
  for (unsigned int i = 0; i < teNum; i++) {
    /// @todo for each TE
    TestCaseExecute(testCaseId, tsiPortList);
  }
}

void freettcn::CH::CComponentHandler::TestCaseExecute(const TciTestCaseIdType &testCaseId, const TriPortIdList &tsiPortList)
{
  tciExecuteTestCase(testCaseId, tsiPortList);
}


void freettcn::CH::CComponentHandler::ConnectReq(const TriPortId &fromPort, const TriPortId &toPort)
{
  Connect(fromPort, toPort);
  
//   fromPort.cmp.te.Connect(TriPortId fromPort, TriPortId toPort);
//   if (fromPort.cmp.te != toPort.cmp.te)
//     toPort.cmp.te.Connect(TriPortId fromPort, TriPortId toPort);
}

void freettcn::CH::CComponentHandler::Connect(const TriPortId &fromPort, const TriPortId &toPort)
{
  tciConnect(fromPort, toPort);
}


void freettcn::CH::CComponentHandler::DisconnectReq(const TriPortId &fromPort, const TriPortId &toPort)
{
  Disconnect(fromPort, toPort);
  
//   fromPort.cmp.te.Disconnect(TriPortId fromPort, TriPortId toPort);
//   if (fromPort.cmp.te != toPort.cmp.te)
//     toPort.cmp.te.Disconnect(TriPortId fromPort, TriPortId toPort);
}

void freettcn::CH::CComponentHandler::Disconnect(const TriPortId &fromPort, const TriPortId &toPort)
{
  tciDisconnect(fromPort, toPort);
}


void freettcn::CH::CComponentHandler::MapReq(const TriPortId &fromPort, const TriPortId &toPort)
{
  Map(fromPort, toPort);
}

void freettcn::CH::CComponentHandler::Map(const TriPortId &fromPort, const TriPortId &toPort)
{
  tciMap(fromPort, toPort);
}

void freettcn::CH::CComponentHandler::UnmapReq(const TriPortId &fromPort, const TriPortId &toPort)
{
  Unmap(fromPort, toPort);
}

void freettcn::CH::CComponentHandler::Unmap(const TriPortId &fromPort, const TriPortId &toPort)
{
  tciUnmap(fromPort, toPort);
}