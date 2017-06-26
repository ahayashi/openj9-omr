/*******************************************************************************
 *
 * (c) Copyright IBM Corp. 2000, 2016
 *
 *  This program and the accompanying materials are made available
 *  under the terms of the Eclipse Public License v1.0 and
 *  Apache License v2.0 which accompanies this distribution.
 *
 *      The Eclipse Public License is available at
 *      http://www.eclipse.org/legal/epl-v10.html
 *
 *      The Apache License v2.0 is available at
 *      http://www.opensource.org/licenses/apache2.0.php
 *
 * Contributors:
 *    Multiple authors (IBM Corp.) - initial implementation and documentation
 ******************************************************************************/

#ifndef OMR_VIRTUALGUARD_INCL
#define OMR_VIRTUALGUARD_INCL

#include <stddef.h>                      // for NULL
#include <stdint.h>                      // for int16_t, int32_t
#include "env/KnownObjectTable.hpp"  // for KnownObjectTable, etc
#include "env/TRMemory.hpp"              // for TR_Memory, etc
#include "env/jittypes.h"                // for uintptrj_t
#include "infra/Assert.hpp"              // for TR_ASSERT
#include "infra/List.hpp"                // for List

class TR_InnerAssumption;
class TR_OpaqueClassBlock;
class TR_VirtualGuardSite;
namespace TR { class Compilation; }
namespace TR { class Node; }
namespace TR { class ResolvedMethodSymbol; }
namespace TR { class SymbolReference; }
namespace TR { class TreeTop; }

enum TR_VirtualGuardKind
   {
   TR_NoGuard,
   TR_ProfiledGuard,
   TR_RemovedProfiledGuard,
   TR_InterfaceGuard,
   TR_RemovedInterfaceGuard,
   TR_AbstractGuard,
   TR_HierarchyGuard,
   TR_NonoverriddenGuard,
   TR_RemovedNonoverriddenGuard,
   TR_SideEffectGuard,
   TR_DummyGuard,
   TR_HCRGuard,
   TR_MutableCallSiteTargetGuard,
   TR_MethodEnterExitGuard,
   TR_DirectMethodGuard,
   TR_InnerGuard,
   TR_ArrayStoreCheckGuard,
   TR_OSRGuard,
   TR_BreakpointGuard
   };

enum TR_VirtualGuardTestType
   {
   TR_DummyTest,
   TR_VftTest,
   TR_MethodTest,
   TR_NonoverriddenTest,
   TR_RubyInlineTest,
   TR_FSDTest /**< used in debugging mode to test if a breakpoint is set for the inlined callee */
   };



class TR_VirtualGuard
   {
   public:

   TR_ALLOC(TR_Memory::VirtualGuard)

   TR_VirtualGuard(
         TR_VirtualGuardTestType test,
         TR_VirtualGuardKind kind,
         TR::Compilation *comp,
         TR::Node *callNode,
         TR::Node *guardNode,
         int16_t calleeIndex,
         int32_t currentSiteIndex,
         TR_OpaqueClassBlock *thisClass = 0);

   // Used for guarded devirtualizations
   TR_VirtualGuard(
         TR_VirtualGuardTestType test,
         TR_VirtualGuardKind kind,
         TR::Compilation *comp,
         TR::Node *callNode,
         TR::Node *guardNode=NULL,
         int32_t currentSiteIndex=-2);

   static TR::Node *createRubyInlineGuard(
         TR_VirtualGuardKind,
         TR::Compilation *,
         int16_t calleeIndex,
         TR::Node *,
         TR::TreeTop *,
         TR_OpaqueClassBlock *);

   static TR::Node *createVftGuard(
         TR_VirtualGuardKind,
         TR::Compilation *,
         int16_t calleeIndex,
         TR::Node *,
         TR::TreeTop *,
         TR_OpaqueClassBlock *);

   static TR::Node *createMethodGuard(
         TR_VirtualGuardKind,
         TR::Compilation *,
         int16_t calleeIndex,
         TR::Node *,
         TR::TreeTop *,
         TR::ResolvedMethodSymbol *,
         TR_OpaqueClassBlock *thisClass);

   static TR::Node *createMethodGuardWithReceiver(
         TR_VirtualGuardKind,
         TR::Compilation *,
         int16_t calleeIndex,
         TR::Node *,
         TR::TreeTop *,
         TR::ResolvedMethodSymbol *,
         TR_OpaqueClassBlock *thisClass,
         TR::Node* receiverNode);

   static TR::Node *createNonoverriddenGuard(
         TR_VirtualGuardKind,
         TR::Compilation *,
         int16_t calleeIndex,
         TR::Node *,
         TR::TreeTop *,
         TR::ResolvedMethodSymbol *,
         bool forInling = true);

   static TR::Node *createMutableCallSiteTargetGuard(
         TR::Compilation * comp,
         int16_t calleeIndex,
         TR::Node * node,
         TR::TreeTop * destination,
         uintptrj_t *mcsObject,
         TR::KnownObjectTable::Index mcsEpoch);

   static TR::Node *createDummyOrSideEffectGuard(TR::Compilation *, TR::Node *, TR::TreeTop *);

   static TR::Node *createSideEffectGuard(TR::Compilation *, TR::Node *, TR::TreeTop *);

   static TR::Node *createAOTInliningGuard(TR::Compilation *, int16_t, TR::Node *, TR::TreeTop *, TR_VirtualGuardKind);

   static TR::Node *createAOTGuard(TR::Compilation *, int16_t, TR::Node *, TR::TreeTop *, TR_VirtualGuardKind);

   static TR::Node *createDummyGuard(TR::Compilation *, int16_t, TR::Node *, TR::TreeTop *);

   static TR_VirtualGuard *createGuardedDevirtualizationGuard(TR_VirtualGuardKind, TR::Compilation *, TR::Node *callNode);

   static TR_VirtualGuard *createArrayStoreCheckGuard(TR::Compilation *, TR::Node *node, TR_OpaqueClassBlock *clazz);

   static TR::Node *createOSRGuard(TR::Compilation *, TR::TreeTop *destination);

   static TR::Node *createHCRGuard(
         TR::Compilation *comp,
         int16_t calleeIndex,
         TR::Node *node,
         TR::TreeTop *destination,
         TR::ResolvedMethodSymbol * symbol,
         TR_OpaqueClassBlock *thisClass);

   static TR::Node *createBreakpointGuard(TR::Compilation * comp, int16_t calleeIndex, TR::Node* callNode, TR::TreeTop * destination, TR::ResolvedMethodSymbol * calleeSymbol);
   static TR::Node *createBreakpointGuardNode(TR::Compilation * comp, int16_t calleeIndex, TR::Node* callNode, TR::TreeTop * destination, TR::ResolvedMethodSymbol * calleeSymbol);

   static void setGuardKind(TR::Node *guard, TR_VirtualGuardKind kind, TR::Compilation * comp);

   bool isNopable()
      {
      switch (_kind)
         {
         case TR_ProfiledGuard:
            return false;
         default:
            return true;
         }
      }

   bool mustBeNoped()
      {
      switch (_kind)
         {
         case TR_MutableCallSiteTargetGuard:
         case TR_InterfaceGuard:
            return true;
         default:
            return false;
         }
      }

   bool indirectsTheThisPointer()
      {
      if (_kind == TR_HCRGuard)
         return false;

      switch (_test)
         {
         case TR_DummyTest:
         case TR_NonoverriddenTest:
            return false;
         default:
            return true;
         }
      }

   int16_t                 getCalleeIndex() { return _calleeIndex; }
   int32_t                 getByteCodeIndex() { return _byteCodeIndex; }
   TR_VirtualGuardTestType getTestType()    { return _test; }
   TR_VirtualGuardKind     getKind()        { return _kind; }
   TR_OpaqueClassBlock    *getThisClass()   { return _thisClass; }
   void                    setThisClass(TR_OpaqueClassBlock *thisClass)   { _thisClass = thisClass; }

   uintptrj_t *mutableCallSiteObject()
      {
      TR_ASSERT(_kind == TR_MutableCallSiteTargetGuard, "mutableCallSiteObject only defined for TR_MutableCallSiteTargetGuard");
      return _mutableCallSiteObject;
      }

   TR::KnownObjectTable::Index mutableCallSiteEpoch()
      {
      TR_ASSERT(_kind == TR_MutableCallSiteTargetGuard, "mutableCallSiteEpoch only defined for TR_MutableCallSiteTargetGuard");
      return _mutableCallSiteEpoch;
      }

   bool                    isInlineGuard()  { return _callNode == NULL; }
   TR::Node                *getCallNode()    { TR_ASSERT(!isInlineGuard(), "assertion failure"); return _callNode; }
   TR_ByteCodeInfo         &getByteCodeInfo() { return _bcInfo; }
   void                    setCannotBeRemoved() { _cannotBeRemoved = true; }
   bool                    canBeRemoved()   { return !_cannotBeRemoved; }

#ifdef J9_PROJECT_SPECIFIC
   TR_Memory *             trMemory()       { return _sites.trMemory(); }
   TR_HeapMemory           trHeapMemory()   { return trMemory(); }

   TR_VirtualGuardSite *addNOPSite();
   List<TR_VirtualGuardSite> &getNOPSites()   { return _sites; }
#endif

   TR::SymbolReference *getSymbolReference() { return _guardedMethod; }
   void setSymbolReference(TR::SymbolReference* sr) { _guardedMethod = sr; }

   void addInnerAssumption(TR_InnerAssumption *a);
   void addInnerAssumption(TR::Compilation *comp, int32_t ordinal, TR_VirtualGuard *guard);
   List<TR_InnerAssumption> &getInnerAssumptions() { return _innerAssumptions; }

   bool                    shouldGenerateChildrenCode() { return _evalChildren; }
   void                    dontGenerateChildrenCode()   { _evalChildren = false; }
   void                    setMergedWithHCRGuard() { _mergedWithHCRGuard=true; _cannotBeRemoved = true; }
   bool                    mergedWithHCRGuard() { return _mergedWithHCRGuard; }

   void                    setMergedWithOSRGuard() { _mergedWithOSRGuard=true; _cannotBeRemoved = true; }
   bool                    mergedWithOSRGuard() { return _mergedWithOSRGuard; }

   int32_t                 getCurrentInlinedSiteIndex() { return _currentInlinedSiteIndex; }
   void                    setCurrentInlinedSiteIndex(int32_t index) { _currentInlinedSiteIndex = index; }

   TR::Node *getGuardNode() { return _guardNode; }
   void setGuardNode(TR::Node *guardNode) { _guardNode = guardNode; }

   private:

#ifdef J9_PROJECT_SPECIFIC
   List<TR_VirtualGuardSite> _sites;
#endif
   TR_VirtualGuardTestType   _test;
   TR_VirtualGuardKind       _kind;
   int16_t                   _calleeIndex;
   int32_t                   _byteCodeIndex;
   TR::SymbolReference       *_guardedMethod;

   // Non-null for guarded-devirtualizations only
   TR::Node                  *_callNode;

   // used for AOT
   TR::Node                  *_guardNode;
   int32_t                   _currentInlinedSiteIndex;

   // used for Interface/Method, Abstract/Method, Hierarchy/Method
   TR_OpaqueClassBlock      *_thisClass;

   bool                      _cannotBeRemoved;
   List<TR_InnerAssumption>  _innerAssumptions;
   bool                      _evalChildren;
   bool                      _mergedWithHCRGuard;
   bool                      _mergedWithOSRGuard;

   // These reference locations are non-null only for MutableCallSiteGuards
   uintptrj_t                *_mutableCallSiteObject;
   TR::KnownObjectTable::Index _mutableCallSiteEpoch;
   TR_ByteCodeInfo           _bcInfo;
   };

#endif
