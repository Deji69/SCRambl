/**********************************************************/
// SCRambl Advanced SCR Compiler/Assembler
// This program is distributed freely under the MIT license
// (See the LICENSE file provided
//	 or copy at http://opensource.org/licenses/MIT)
/**********************************************************/
#pragma once

namespace SCRambl
{
	template<class, typename> class CTree;
	template<class> class CList;

	// made for adding as a parent class for quick linked list integration - don't use without prettying it up
	/*template<class T>
	class CLList
	{
	T		*	m_pNext;
	T		*	m_pPrev;

	public:
	inline void		Next(T *p)				{ m_pNext = p; }
	inline void		Prev(T *p)				{ m_pPrev = p; }

	void		InsertNext(const T *p)
	{
	p->Next(Next());
	p->Prev(this);
	Next()->Prev(p);
	Next(p);
	}
	void		InsertPrev(const T *p)
	{
	p->Next(this);
	p->Prev(Prev());
	Prev()->Next(p);
	Prev(p);
	}

	inline T		*	Next() const		{ return m_pNext; }
	inline T		*	Prev() const		{ return m_pPrev; }
	};*/

	template<class T> class CList;
	template<class T> class CLinkPtr;

	// A link from a linked-list
	template<class T>
	class CLink
	{
		friend CList<T>;
		friend CLinkPtr<T>;
		typedef CLink<T> 	LLink;
		typedef LLink	*	LLinkPtr;
		typedef LLink	&	RLink;

		LLinkPtr		m_pNext;
		LLinkPtr		m_pPrev;
		T			m_pThis;

		void			Set(T p)				{ m_pThis = p; }
		void			Next(LLinkPtr p)		{ m_pNext = p; }
		void			Prev(LLinkPtr p)		{ m_pPrev = p; }

		CLink() : m_pNext(nullptr), m_pPrev(nullptr)
		{
		}
		CLink(T const & p, LLinkPtr q = nullptr, LLinkPtr bad = nullptr)
			: m_pThis(p), m_pNext(q ? q->Next() : bad), m_pPrev(q ? q : bad)
		{
			if (m_pPrev != bad)
			{
				if (m_pPrev->Next() != bad) q->Next()->Prev(this);
				m_pPrev->Next(this);
			}
		}

	public:
		~CLink()
		{
			if (m_pNext != this) delete m_pNext;
		}

		inline T				&	Get()					{ return m_pThis; }
		inline LLinkPtr			Next()					{ return m_pNext; }
		inline LLinkPtr			Prev()					{ return m_pPrev; }
	};

	// Smart pointers
	template<class T>
	class CPtr
	{
		typedef CPtr<T>		&	RPtr;
		T					*	m_pPtr;

	public:
		CPtr() : m_pPtr(nullptr)
		{
		}
		CPtr(T & obj) : m_pPtr(&obj)
		{
		}

		virtual ~CPtr()
		{
			if (m_pPtr) delete m_pPtr;
		}

		inline RPtr			operator=(const RPtr o)
		{
			m_pPtr = o.m_pPtr;
			return *this;
		}
		inline RPtr			operator=(const T &o)
		{
			m_pPtr = &o;
			return *this;
		}

		inline T &		operator*() const						{ return *m_pPtr; }
		inline T &		operator->() const						{ return *m_pPtr; }
		inline bool		operator==(const T& a) const				{ return m_pPtr == &a; }
		inline bool		operator!=(const T& a) const				{ return !(*this == a); }
		inline bool		operator==(const RPtr a) const			{ return m_pPtr == a.m_pPtr; }
		inline bool		operator!=(const RPtr a) const			{ return !(*this == a); }
	};

	// Linked-list iterators
	template<class T>
	class CLinkPtr		// TODO: inherit CPtr?
	{
		friend CList<T>;
		typedef CLink<T>			LLink;
		typedef LLink		*	LLinkPtr;
		typedef LLink		&	RLink;
		typedef CLinkPtr<T>		ILink;
		typedef CLinkPtr<T>	&	IRLink;

		LLinkPtr				m_pLink;

	public:
		CLinkPtr() : m_pLink(nullptr)
		{
		}
		CLinkPtr(RLink Link) : m_pLink(&Link)
		{
		}
		~CLinkPtr()
		{
		}

		// assign
		inline IRLink		operator=(const IRLink o)
		{
			m_pLink = o.m_pLink;
			return *this;
		}
		inline IRLink		operator=(const RLink o)
		{
			m_pLink = &o;
			return *this;
		}

		// iterate
		inline IRLink		operator++()
		{
			m_pLink = m_pLink->Next();
			return *this;
		}
		inline IRLink		operator--()
		{
			m_pLink = m_pLink->Prev();
			return *this;
		}
		inline ILink		operator++(int)
		{
			ILink it = *this;
			m_pLink = m_pLink->Next();
			return it;
		}
		inline ILink		operator--(int)
		{
			ILink it = *this;
			m_pLink = m_pLink->Prev();
			return it;
		}

		// smartly point
		inline T &		operator*() const						{ return m_pLink->Get(); }
		inline T &		operator->() const						{ return m_pLink->Get(); }

		// smartly compare
		inline bool		operator==(const RLink a) const			{ return m_pLink == &a; }
		inline bool		operator!=(const RLink a) const			{ return !(*this == a); }
		inline bool		operator==(const IRLink a) const			{ return m_pLink == a.m_pLink; }
		inline bool		operator!=(const IRLink a) const			{ return !(*this == a); }
	};

	// Linked-list (bi-directional)
	template<class T>
	class CList
	{
	public:
		typedef CLink<T>			LLink;
		typedef LLink		*	LLinkPtr;
		typedef LLink		&	RLink;
		typedef CLinkPtr<T>		ILink;
		typedef CLinkPtr<T>	&	IRLink;

	private:
		LLinkPtr		m_pFirst;
		LLinkPtr		m_pLast;
		LLinkPtr		m_pEnd;

	public:
		CList() : m_pFirst(nullptr)
		{
			Init();
		}
		~CList()
		{
			Clear();
		}

		void	 Init()
		{
			if (!m_pFirst || m_pFirst == m_pEnd)
			{
				m_pFirst = nullptr;
				m_pLast = nullptr;
				m_pEnd = new LLink();
				m_pEnd->Next(m_pEnd);
				m_pEnd->Prev(m_pEnd);
			}
		}

		void	 Clear()
		{
			if (m_pFirst)
				delete m_pFirst;
			m_pFirst = nullptr;
			m_pLast = nullptr;
		}
		void	 Add(T const& p)
		{
			auto pLast = m_pLast;
			m_pLast = new LLink(p, pLast, m_pEnd);
			if (!m_pFirst) m_pFirst = m_pLast;
		}
		void	 Drop(ILink const& it)
		{
			if (it == Begin())
				m_pFirst = it.m_pLink->Next();
			if (it == End())
				m_pLast = it.m_pLink->Prev();
			it.m_pLink->Next()->Prev(it.m_pLink->Prev());
			it.m_pLink->Prev()->Next(it.m_pLink->Next());
			m_pEnd->Next(m_pEnd);
			m_pEnd->Prev(m_pEnd);
		}

		inline ILink Begin() const
		{
			return m_pFirst ? *m_pFirst : *m_pEnd;
		}
		inline ILink End() const
		{
			return m_pLast ? *m_pLast : *m_pEnd;
		}
		inline ILink Bad() const
		{
			return *m_pEnd;
		}
	};

	// Binary tree nodes
	template<class T, typename K = int>
	class CNode
	{
		friend			CTree<T, K>;
		K				m_nKey;
		T				m_pObject;
		CNode<T, K>	*	m_pLeft;
		CNode<T, K>	*	m_pRight;
		CNode<T, K>	*	m_pRoot;

		CNode(K key, T object)
		{
			m_nKey = key;
			m_pObject = object;
			m_pLeft = nullptr;
			m_pRight = nullptr;
			m_pRoot = nullptr;
		}
		~CNode()
		{
			if (m_pLeft) delete m_pLeft;
			if (m_pRight) delete m_pRight;
		}
		inline void SetLeft(CNode<T, K> * pNode)
		{
			if (m_pLeft)
			{
				if (pNode->m_nKey < m_pLeft->m_nKey)
					m_pLeft->SetLeft(pNode);
				else
					m_pLeft->SetRight(pNode);
			}
			else
			{
				m_pLeft = pNode;
				pNode->m_pRoot = this;
			}
		}
		inline void SetRight(CNode<T, K> * pNode)
		{
			if (m_pRight)
			{
				if (pNode->m_nKey < m_pRight->m_nKey)
					m_pRight->SetLeft(pNode);
				else
					m_pRight->SetRight(pNode);
			}
			else
			{
				m_pRight = pNode;
				pNode->m_pRoot = this;
			}
		}

		void Drop()
		{
			if (!m_pLeft && !m_pRight)
			{
				if (m_pRoot->m_pLeft == this)
					m_pRoot->m_pLeft = nullptr;
				else
					m_pRoot->m_pRight = nullptr;
			}
			else
			{
				if (m_pRoot->m_pLeft == this)
				{
					if (m_pRight) m_pRoot->SetLeft(m_pRight);
					if (m_pLeft) m_pRoot->SetLeft(m_pLeft);
				}
				else
				{
					if (m_pRight) m_pRoot->SetRight(m_pRight);
					if (m_pLeft) m_pRoot->SetRight(m_pLeft);
				}
			}
			m_pLeft = nullptr;
			m_pRight = nullptr;
			delete this;
		}
	};

	// Binary tree list
	template<class T, typename K>
	class CTree
	{
		CNode<T, K>	*	m_pBase;
		CNode<T, K>	*	m_pCurrentNode;

		// Add to sub-left of the current node if it's lower, right if it's higher
		void Add(K key, T object, CNode<T, K> * node)
		{
			if (key < node->m_nKey)
			{
				if (node->m_pLeft) Add(key, object, node->m_pLeft);
				else
				{
					node->m_pLeft = new CNode<T, K>(key, object);
					node->m_pLeft->m_pRoot = node;
				}
			}
			else
			{
				if (node->m_pRight) Add(key, object, node->m_pRight);
				else
				{
					node->m_pRight = new CNode<T, K>(key, object);
					node->m_pRight->m_pRoot = node;
				}
			}
		}

		// Search a branch for the key
		CNode<T, K> * Search(K key, CNode<T, K>* node) const
		{
			if (node)
			{
				if (key == node->m_nKey) return node;
				return key < node->m_nKey ? Search(key, node->m_pLeft) : Search(key, node->m_pRight);
			}
			return nullptr;
		}
		CNode<T, K> * SearchForHighest(CNode<T, K>* node)
		{
			if (node) return node->m_pRight ? SearchForHighest(node->m_pRight) : node;
			return nullptr;
		}
		CNode<T, K> * SearchForLowest(CNode<T, K>* node)
		{
			if (node) return node->m_pLeft ? SearchForLowest(node->m_pLeft) : node;
			return nullptr;
		}
		CNode<T, K> * SearchForHighest(K max, CNode<T, K>* node)
		{
			if (node)
			{
				if (node->m_nKey < max)
				{
					if (node->m_pRight) return node->m_pRight->m_nKey < max ? SearchForHighest(max, node->m_pRight) : node;
					return node;
				}
				return node->m_pLeft ? SearchForHighest(max, node->m_pLeft) : nullptr;
			}
			return nullptr;
		}
		CNode<T, K> * SearchForLowest(K min, CNode<T, K>* node)
		{
			if (node)
			{
				if (node->m_nKey >= min)
				{
					if (node->m_pLeft) return node->m_pLeft->m_nKey >= min ? SearchForLowest(min, node->m_pLeft) : node;
					return node;
				}
				return node->m_pRight ? SearchForLowest(min, node->m_pRight) : nullptr;
			}
			return nullptr;
		}

	public:
		CTree()
		{
			m_pBase = nullptr;
			m_pCurrentNode = nullptr;
		}
		~CTree()
		{
			if (m_pBase) delete m_pBase;
		}

		void Clear()
		{
			delete m_pBase;
			m_pBase = nullptr;
		}
		void ClearNoDelete()
		{
			m_pBase = nullptr;
		}
		void Add(K key, T object)
		{
			if (m_pBase) Add(key, object, m_pBase);
			else m_pBase = new CNode<T, K>(key, object);
		}
		void Drop(K key, T object)
		{
			auto pNode = Search(key, m_pBase);
			while (pNode && pNode->m_pObject != object)
				pNode = pNode->m_pRight->m_nKey == key ? pNode->m_pRight : nullptr;
			if (pNode) pNode->Drop();
		}

		T Get(K key)
		{
			m_pCurrentNode = Search(key, m_pBase);
			return m_pCurrentNode ? m_pCurrentNode->m_pObject : nullptr;
		}
		T Get(K key) const
		{
			auto pCurrentNode = Search(key, m_pBase);
			return pCurrentNode ? pCurrentNode->m_pObject : nullptr;
		}
		T GetHighest()
		{
			m_pCurrentNode = SearchForHighest(m_pBase);
			return m_pCurrentNode ? m_pCurrentNode->m_pObject : nullptr;
		}
		T GetLowest()
		{
			m_pCurrentNode = SearchForHighest(m_pBase);
			return m_pCurrentNode ? m_pCurrentNode->m_pObject : nullptr;
		}
		T GetHighest(K max)
		{
			m_pCurrentNode = max ? SearchForHighest(max, m_pBase) : SearchForHighest(m_pBase);
			return m_pCurrentNode ? m_pCurrentNode->m_pObject : nullptr;
		}
		T GetLowest(K min)
		{
			m_pCurrentNode = min ? SearchForHighest(min, m_pBase) : SearchForHighest(m_pBase);
			return m_pCurrentNode ? m_pCurrentNode->m_pObject : nullptr;
		}
		T Next()
		{
			if (!m_pCurrentNode || !m_pCurrentNode->m_pRight) return nullptr;
			if (m_pCurrentNode->m_pRight->m_nKey != m_pCurrentNode->m_nKey) return nullptr;
			m_pCurrentNode = m_pCurrentNode->m_pRight;
			return m_pCurrentNode->m_pObject;
		}

		T& operator[](K key)
		{
			return Get(key);
		}
		const T& operator[](K key) const
		{
			return Get(key);
		}
	};
}