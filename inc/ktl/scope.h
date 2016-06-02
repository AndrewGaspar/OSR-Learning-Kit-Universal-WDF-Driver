#pragma once

#include "utility.h"

#ifndef SCOPE_EXIT_CODE_SEGMENT
#define SCOPE_EXIT_CODE_SEGMENT "PAGE"
#endif

namespace ktl
{
    namespace details
    {
        template<typename Functor>
        class __declspec(code_seg(SCOPE_EXIT_CODE_SEGMENT)) ScopeExitImpl
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

            ScopeExitImpl & operator=(ScopeExitImpl&& other)
            {
                m_isDismissed = other.m_isDismissed;
                m_scopeExit = ktl::move(other.m_scopeExit);

                other.m_isDismissed = true;

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
    auto __declspec(code_seg(SCOPE_EXIT_CODE_SEGMENT)) make_scope_exit(F&& f)
    {
        return details::ScopeExitImpl<F>(ktl::move(f));
    }
}