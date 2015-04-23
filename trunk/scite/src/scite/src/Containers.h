#ifndef CONTAINERS_H
#define CONTAINERS_H

struct T_CONTAINERS_POSITION { };
typedef T_CONTAINERS_POSITION* T_POSITION;
#define BEFORE_START_T_POSITION ((T_POSITION)-1L)

template<class TYPE, class ARG_TYPE>
class TList
{
protected:
	struct Node	{
		Node* pNext;
		Node* pPrev;
		TYPE data;
	};
	
public:

	TList() {
		m_nCount = 0;
		m_pNodeHead = m_pNodeTail = ((Node *)0);
	}

	~TList() {
		RemoveAll();
	}
	
	int GetCount() const {
		return m_nCount;
	}
	bool IsEmpty() const {
		return m_nCount == 0;
	}

	TYPE& GetHead() {
		return m_pNodeHead->data;
	}
	TYPE GetHead() const {
		return m_pNodeHead->data;
	}
	TYPE& GetTail() {
		return m_pNodeTail->data;
	}
	TYPE GetTail() const {
		return m_pNodeTail->data;
	}

	// get head or tail (and remove it) - don't call on empty list !
	TYPE RemoveHead() {
		Node* pOldNode = m_pNodeHead;
		TYPE returnValue = pOldNode->data;

		m_pNodeHead = pOldNode->pNext;
		if (m_pNodeHead != ((Node *)0))
			m_pNodeHead->pPrev = ((Node *)0);
		else
			m_pNodeTail = ((Node *)0);
		FreeNode(pOldNode);
		return returnValue;
	}
	TYPE RemoveTail() {
		Node* pOldNode = m_pNodeTail;
		TYPE returnValue = pOldNode->data;

		m_pNodeTail = pOldNode->pPrev;
		if (m_pNodeTail != ((Node *)0))
			m_pNodeTail->pNext = ((Node *)0);
		else
			m_pNodeHead = ((Node *)0);
		FreeNode(pOldNode);
		return returnValue;
	}

	// add before head or after tail
	T_POSITION AddHead(ARG_TYPE newElement) {
		Node* pNewNode = NewNode(((Node *)0), m_pNodeHead);
		pNewNode->data = newElement;
		if (m_pNodeHead != ((void *)0))
			m_pNodeHead->pPrev = pNewNode;
		else
			m_pNodeTail = pNewNode;
		m_pNodeHead = pNewNode;
		return (T_POSITION) pNewNode;
	}
	T_POSITION AddTail(ARG_TYPE newElement) {
		Node* pNewNode = NewNode(m_pNodeTail, ((Node *)0));
		pNewNode->data = newElement;
		if (m_pNodeTail != ((Node *)0))
			m_pNodeTail->pNext = pNewNode;
		else
			m_pNodeHead = pNewNode;
		m_pNodeTail = pNewNode;
		return (T_POSITION) pNewNode;
	}

	// remove all elements
	void RemoveAll() {
		// destroy elements
		Node* pNode = m_pNodeHead;
		while (pNode) {
			m_pNodeTail = pNode->pNext;
			delete pNode;
			pNode = m_pNodeTail;
		}
		m_nCount = 0;
		m_pNodeHead = m_pNodeTail = ((Node *)0);
	}

	// iteration
	T_POSITION GetHeadPosition() const { 
		return (T_POSITION) m_pNodeHead; 
	}
	T_POSITION GetTailPosition() const {
		return (T_POSITION) m_pNodeTail;
	}
	TYPE& GetNext(T_POSITION& rPosition) { // return *Position++
		Node* pNode = (Node*) rPosition;
		rPosition = (T_POSITION) pNode->pNext;
		return pNode->data;
	}
	TYPE GetNext(T_POSITION& rPosition) const { // return *Position++
		Node* pNode = (Node*) rPosition;
		rPosition = (T_POSITION) pNode->pNext;
		return pNode->data;
	}
	TYPE& GetPrev(T_POSITION& rPosition) { // return *Position--
		Node* pNode = (Node*) rPosition;
		rPosition = (T_POSITION) pNode->pPrev;
		return pNode->data;
	}
	TYPE GetPrev(T_POSITION& rPosition) const { // return *Position--
		Node* pNode = (Node*) rPosition;
		rPosition = (T_POSITION) pNode->pPrev;
		return pNode->data;
	}

	// getting/modifying an element at a given position
	TYPE& GetAt(T_POSITION position) {
		Node* pNode = (Node*) position;
		return pNode->data;
	}
	TYPE GetAt(T_POSITION position) const {
		Node* pNode = (Node*) position;
		return pNode->data;
	}
	void SetAt(T_POSITION pos, ARG_TYPE newElement) {
		Node* pNode = (Node*) pos;
		pNode->data = newElement; 
	}
	void RemoveAt(T_POSITION position) {
		Node* pOldNode = (Node*) position;
		// remove pOldNode from list
		if (pOldNode == m_pNodeHead)
			m_pNodeHead = pOldNode->pNext;
		else
			pOldNode->pPrev->pNext = pOldNode->pNext;
		if (pOldNode == m_pNodeTail)
			m_pNodeTail = pOldNode->pPrev;
		else
			pOldNode->pNext->pPrev = pOldNode->pPrev;
		FreeNode(pOldNode);
	}

	// inserting before or after a given position
	T_POSITION InsertBefore(T_POSITION position, ARG_TYPE newElement) {
		if (position == ((void *)0))
			return AddHead(newElement); // insert before nothing -> head of the list
		// Insert it before position
		Node* pOldNode = (Node*) position;
		Node* pNewNode = NewNode(pOldNode->pPrev, pOldNode);
		pNewNode->data = newElement;
		if (pOldNode->pPrev != ((Node *)0))
			pOldNode->pPrev->pNext = pNewNode;
		else
			m_pNodeHead = pNewNode;
		pOldNode->pPrev = pNewNode;
		return (T_POSITION) pNewNode;
	}
	T_POSITION InsertAfter(T_POSITION position, ARG_TYPE newElement) {
		if (position == ((void *)0))
			return AddTail(newElement); // insert after nothing -> tail of the list
		// Insert it before position
		Node* pOldNode = (Node*) position;
		Node* pNewNode = NewNode(pOldNode, pOldNode->pNext);
		pNewNode->data = newElement;
		if (pOldNode->pNext != ((Node *)0))
			pOldNode->pNext->pPrev = pNewNode;
		else
			m_pNodeTail = pNewNode;
		pOldNode->pNext = pNewNode;
		return (T_POSITION) pNewNode;
	}

	// helper functions (note: O(n) speed)
	// defaults to starting at the HEAD, return NULL if not found
	T_POSITION Find(ARG_TYPE searchValue, T_POSITION startAfter = (T_POSITION)0) const {
		Node* pNode = (Node*) startAfter;
		if (pNode == ((Node *)0))
			pNode = m_pNodeHead;  // start at head
		else
			pNode = pNode->pNext;  // start after the one specified
		for (; pNode != ((Node *)0); pNode = pNode->pNext)
			if (pNode->data==searchValue)
				return (T_POSITION)pNode;
		return ((T_POSITION)0);
	}
	// get the 'nIndex'th element (may return NULL)
	T_POSITION FindIndex(int nIndex) const {
		if (nIndex >= m_nCount || nIndex < 0)
			return (T_POSITION)0;  // went too far
		Node* pNode = m_pNodeHead;
		while (nIndex--)
			pNode = pNode->pNext;
		return (T_POSITION) pNode;
	}

protected:
	Node* m_pNodeHead;
	Node* m_pNodeTail;
	int m_nCount;

	Node* NewNode(Node* pPrev, Node* pNext) {
		Node* pNode = new Node;
		pNode->pPrev = pPrev;
		pNode->pNext = pNext;
		m_nCount++;
		return pNode;
	}
	void FreeNode(Node*pNode) {
		delete pNode;
		m_nCount--;
		// if no more elements, cleanup completely
		if (m_nCount == 0)
			RemoveAll();
	}
};

template<class TYPE, class ARG_TYPE>
class TArray
{
public:
	TArray() {};

	int GetSize() const {
		return m_ArrayData.GetCount();
	}
	
	// Clean up
	void RemoveAll() {
		m_ArrayData.RemoveAll();
	}
	
	// Accessing elements
	TYPE GetAt(int nIndex) const {
		return m_ArrayData.GetAt(m_ArrayData.FindIndex(nIndex));
	}
	TYPE& ElementAt(int nIndex) {
		return m_ArrayData.GetAt(m_ArrayData.FindIndex(nIndex));
	}
	void SetAt(int nIndex, ARG_TYPE newElement) {
		m_ArrayData.SetAt(m_ArrayData.FindIndex(nIndex),newElement);
	}
	
	// Potentially growing the array
	int Add(ARG_TYPE newElement) {
		m_ArrayData.AddTail(newElement);
		return m_ArrayData.GetCount();
	}
	
	// overloaded operator helpers
	TYPE operator[](int nIndex) const {
		return m_ArrayData.GetAt(m_ArrayData.FindIndex(nIndex));
	}
	TYPE& operator[](int nIndex) {
		return m_ArrayData.GetAt(m_ArrayData.FindIndex(nIndex));
	}
	
	// Operations that move elements around
	void InsertAt(int nIndex, ARG_TYPE newElement, int nCount = 1) {
			for (int i=0; i<nCount; i++)
					m_ArrayData.InsertBefore(m_ArrayData.FindIndex(nIndex),newElement); 
	}
	void RemoveAt(int nIndex, int nCount = 1) {
		for (int i=0; i<nCount; i++)
			m_ArrayData.RemoveAt(m_ArrayData.FindIndex(nIndex));
	}

protected:
	TList<TYPE, ARG_TYPE> m_ArrayData;
};

template<class KEY, class ARG_KEY, class VALUE, class ARG_VALUE>
class TMap
{
protected:
	// Association
	struct Association {
		Association* pNext;
		unsigned int nHashValue;  // needed for efficient iteration
		KEY key;
		VALUE value;
	};
public:
	
	TMap() {
		m_pHashTable = 0;
		m_nHashTableSize = 17;  // default size
		m_nCount = 0;
		m_pFreeList = 0;
	}
	~TMap() {
		RemoveAll();
	}
	
	int GetCount() const {
		return m_nCount;
	}
	bool IsEmpty() const {
		return m_nCount == 0;
	}
		
	// Lookup
	bool Lookup(ARG_KEY key, VALUE& rValue) {
		UINT nHash;
		Association* pAssoc = GetAssocAt(key, nHash);
		if (pAssoc == 0)
			return false;  // not in map
		rValue = pAssoc->value;
		return true;
	}
		
	// Operations
	// Lookup and add if not there
	VALUE& operator[](ARG_KEY key) {
		UINT nHash;
		Association* pAssoc;
		if ((pAssoc = GetAssocAt(key, nHash)) == 0)
		{
			if (m_pHashTable == 0)
				InitHashTable(m_nHashTableSize);
			
			// it doesn't exist, add a new Association
			pAssoc = NewAssoc();
			pAssoc->nHashValue = nHash;
			pAssoc->key = key;
			// 'pAssoc->value' is a constructed object, nothing more
			
			// put into hash table
			pAssoc->pNext = m_pHashTable[nHash];
			m_pHashTable[nHash] = pAssoc;
		}
		return pAssoc->value;  // return new reference
	}
		
	// add a new (key, value) pair
	void SetAt(ARG_KEY key, ARG_VALUE newValue) { 
		(*this)[key] = newValue; 
	}
	
	// removing existing (key, ?) pair
	bool RemoveKey(ARG_KEY key) {
		if (m_pHashTable == 0)
			return false;  // nothing in the table
		
		Association** ppAssocPrev;
		ppAssocPrev = &m_pHashTable[HashKey(key) % m_nHashTableSize];
		
		Association* pAssoc;
		for (pAssoc = *ppAssocPrev; pAssoc != 0; pAssoc = pAssoc->pNext)
		{
			if (pAssoc->key == key)
			{
				// remove it
				*ppAssocPrev = pAssoc->pNext;  // remove from list
				FreeAssoc(pAssoc);
				return true;
			}
			ppAssocPrev = &pAssoc->pNext;
		}
		return false;  // not found
	}
	void RemoveAll() {
		if (m_pHashTable != 0)
		{
			// destroy elements (values and keys)
			for (unsigned int nHash = 0; nHash < m_nHashTableSize; nHash++)	{
				Association* pAssoc = m_pHashTable[nHash];
				Association* pAssocNext;
				while (pAssoc) {
					pAssocNext = pAssoc->pNext;
					delete pAssoc;
					pAssoc = pAssocNext;
				}
			}
		}
		// free hash table
		delete[] m_pHashTable;
		m_pHashTable = 0;
		m_nCount = 0;
		m_pFreeList = 0;
	}
		
	// iterating all (key, value) pairs
	T_POSITION GetStartPosition() const { 
		return (m_nCount == 0) ? 0 : BEFORE_START_T_POSITION; 
	}
	void GetNextAssoc(T_POSITION& rNextPosition, KEY& rKey, VALUE& rValue) const {
		Association* pAssocRet = (Association*)rNextPosition;
		
		if (pAssocRet == (Association*) BEFORE_START_T_POSITION)
		{
			// find the first association
			for (unsigned int nBucket = 0; nBucket < m_nHashTableSize; nBucket++)
				if ((pAssocRet = m_pHashTable[nBucket]) != NULL)
					break;
		}
		
		// find next association
		Association* pAssocNext;
		if ((pAssocNext = pAssocRet->pNext) == 0)
		{
			// go to next bucket
			for (unsigned int nBucket = pAssocRet->nHashValue + 1;
			nBucket < m_nHashTableSize; nBucket++)
				if ((pAssocNext = m_pHashTable[nBucket]) != 0)
					break;
		}
		
		rNextPosition = (T_POSITION) pAssocNext;
		
		// fill in return data
		rKey = pAssocRet->key;
		rValue = pAssocRet->value;
	}
	
	// advanced features for derived classes
	unsigned int GetHashTableSize() const { 
		return m_nHashTableSize; 
	}
	void InitHashTable(unsigned int hashSize, bool bAllocNow = true) {
		if (m_pHashTable != 0)
		{
			// free hash table
			delete[] m_pHashTable;
			m_pHashTable = 0;
		}
		
		if (bAllocNow)
		{
			m_pHashTable = new Association* [hashSize];
			memset(m_pHashTable, 0, sizeof(Association*) * hashSize);
		}
		m_nHashTableSize = hashSize;
	}
	
	// Implementation
protected:
	Association** m_pHashTable;
	unsigned int m_nHashTableSize;
	int m_nCount;
	Association* m_pFreeList;
	
	Association* NewAssoc() {
		Association* pAssoc = new Association;
		pAssoc->pNext = m_pFreeList;
		m_pFreeList = pAssoc;
		m_nCount++;
		return pAssoc;
	}

	void FreeAssoc(Association* pAssoc) {
		delete pAssoc;
		m_nCount--;
		// if no more elements, cleanup completely
		if (m_nCount == 0)
			RemoveAll();
	}
	Association* GetAssocAt(ARG_KEY key, unsigned int& nHash) {
		nHash = HashKey(key) % m_nHashTableSize;
		
		if (m_pHashTable == 0)
			return 0;
		
		// see if it exists
		Association* pAssoc;
		for (pAssoc = m_pHashTable[nHash]; pAssoc != 0; pAssoc = pAssoc->pNext)
		{
			if (pAssoc->key == key)
				return pAssoc;
		}
		return 0;
	}
	unsigned int HashKey(ARG_KEY key) {
		// default identity hash - works for most primitive values
		return ((unsigned int)(void*)(unsigned long)key) >> 4;
	}
};

#endif
