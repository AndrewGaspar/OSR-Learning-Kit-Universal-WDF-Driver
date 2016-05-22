#pragma once

#include "utility.h"

namespace ktl
{
    namespace details
    {
        template<typename Functor>
        class ScopeExitImpl
        {
            Functor m_scopeExit;
            bool m_isDismissed = false;
        public:
            ScopeExitImpl(Functor&& f) : m_scopeExit(ktl::move(f))
            {

            }

            ScopeExitImpl(ScopeExitImpl const &) = delete;
            ScopeExitImpl & operator=(ScopeExitImpl const &) = delete;

            ScopeExitImpl(ScopeExitImpl&& other) : m_isDismissed(other.m_isDismissed), m_scopeExit(ktl::move(other.m_scopeExit))
            {
                other.m_isDismissed = true;
            }

            ScopeExitImpl & operator=(ScopeExitImpl&&)
            {
                m_isDismissed = other.m_isDismissed;
                m_scopeExit = other.m_scopeExit;

                m_isDismissed = true;

                return *this;
            }

            void Dismiss()
            {
                m_isDismissed = true;
            }

            ~ScopeExitImpl()
            {
                if (!m_isDismissed)
                {
                    m_scopeExit();
                }
            }
        };
    }

    template<typename F>
    auto ScopeExit(F&& f)
    {
        return details::ScopeExitImpl<decltype(f)>(ktl::move(f));
    }
}