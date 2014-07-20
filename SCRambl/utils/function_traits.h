/**********************************************************/
// SCRambl Advanced SCR Compiler/Assembler
// This program is distributed freely under the MIT license
// (See the LICENSE file provided
//	 or copy at http://opensource.org/licenses/MIT)
/**********************************************************/
#pragma once
#include <functional>

// get the call address of a functor
template <typename Function>
struct function_traits : public function_traits<decltype(&Function::operator())>
{};

// split up the information provided by a function pointer type (arg-less)
template <typename ClassType, typename ReturnType>
struct function_traits<ReturnType(ClassType::*)() const>
{
	typedef ReturnType(*pointer)();
	typedef std::function<ReturnType()> function;
};

// split up the information provided by a function pointer type
template <typename ClassType, typename ReturnType, typename... Args>
struct function_traits<ReturnType(ClassType::*)(Args...) const>
{
	typedef ReturnType(*pointer)(Args...);
	typedef std::function<ReturnType(Args...)> function;
};

// convert a lambda to a function pointer
template <typename Function>
typename function_traits<Function>::pointer to_function_pointer(Function& lambda)
{
	return static_cast<typename function_traits<Function>::pointer>(lambda);
}

// convert a lambda to a function
template <typename Function>
typename function_traits<Function>::function to_function(Function& lambda)
{
	return static_cast<typename function_traits<Function>::function>(lambda);
}