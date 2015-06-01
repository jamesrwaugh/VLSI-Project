#ifndef UTILITY_H
#define UTILITY_H
#include <vector>
#include <string>
#include <iostream>

/* Improved Split funtion that skips over consecutive delimiters, 
 * and splits on more than one delimiter (Used for space and tab) */
std::vector<std::string> Split(const std::string& target, const std::string& delims);

//Remove whitespace before and after a string
void trim(std::string& s);

//Remove a # comment from the end of a string
void removeComments(std::string& s);

/* A drop-in replacement for std::getline that combines multiple lines ending in "\"
 * together into a single line, removes comments, and removes whitespace */
std::istream& getline_fixed(std::istream& is, std::string& line);

//roger - use this for printing a string on a single line
template <typename printType>
void println(printType const &str)
{ 
    std::cout<<str<<std::endl; 
}

template <typename printType>
void print(printType const &str) 
{ 
    std::cout<<str; 
}

template <typename printType>
void printVector(std::vector<printType> const &elements)
{
    for(const auto& e : elements) { println(e); }
}

//Provides a << operator for any standard container
template<typename T, class = typename std::enable_if<
	!std::is_same<T,std::string>::value && std::is_class<T>::value>::type>
std::ostream& operator<<(std::ostream& os, const T& cntr)
{
	os << '[';
	if(not(cntr.empty())) {
		auto end = std::prev(cntr.end());
		for(auto it = cntr.begin(); it != end; ++it)
			os << *it << ",";
		os << *end;
	}
	os << ']';

	return os;
}

#endif
