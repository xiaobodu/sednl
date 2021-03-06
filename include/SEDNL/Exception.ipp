// SEDNL - Copyright (c) 2013 Jeremy S. Cochoy
//
// This software is provided 'as-is', without any express or implied warranty.
// In no event will the authors be held liable for any damages arising from
// the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
//
//     1. The origin of this software must not be misrepresented; you must not
//        claim that you wrote the original software. If you use this software
//        in a product, an acknowledgment in the product documentation would
//        be appreciated but is not required.
//
//     2. Altered source versions must be plainly marked as such, and must not
//        be misrepresented as being the original software.
//
//     3. This notice may not be removed or altered from any source
//        distribution.

#ifndef EXCEPTION_IPP_
#define EXCEPTION_IPP_

namespace SedNL
{

template<typename T>
T TemplateException<T>::get_type()
{
    return m_type;
}

template<typename T>
const char* TemplateException<T>::get_message()
{
    return m_msg;
}

template<typename T>
TemplateException<T>::TemplateException(T type)
    :m_type(type), m_msg(nullptr)
{}

template<typename T>
TemplateException<T>::TemplateException(T type, const char* msg)
    :m_type(type), m_msg(msg)
{}

} // namespace SedNL

#endif /* !EXCEPTION_IPP_ */
