/* sdsl - succinct data structures library
    Copyright (C) 2012 Simon Gog

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see http://www.gnu.org/licenses/ .
*/
/*! \file csa_alphabet_strategy.hpp
    \brief csa_alphabet_strategy.hpp includes different strategy classes for representing an alphabet of a CSA.
	\author Simon Gog
*/

#ifndef INCLUDED_CSA_ALPHABET_STRATEGY
#define INCLUDED_CSA_ALPHABET_STRATEGY

/* A alphabet strategy provides the following features:
 *   * Member `sigma` which contains the size (=number of unique symbols) of the alphabet.
 *   * Method `is_present(char_type c)` which indicates if character c occurs in the text.
 *   * Container `char2comp` which maps a symbol to a number [0..sigma-1]. The alphabetic
 *     order is preserved.
 *   * Container `comp2char` which is the inverse mapping of char2comp.
 *   * Container `C` contains the cumulative counts of occurrences. C[i] is the cumulative
 *     count of occurrences of symbols `comp2char[0]` to `comp2char[i-1]` in the text.
 *   * Typedefs for the four above members:
 *       * char2comp_type
 *       * comp2char_type
 *       * C_type
 *       * sigma_type
 *   * Constructor. Takes a int_vector_file_buffer<8, size_type_class> for byte-alphabets
 *     and int_vector_file_buffer<0, size_type_class> for integer-alphabets. 
 */

#include "int_vector.hpp"

namespace sdsl{

// forward declarations

class byte_alphabet_strategy;

template<class bit_vector_type     = bit_vector,
	     class rank_support_type   = typename bit_vector_type::rank_1_type,
	     class select_support_type = typename bit_vector_type::select_1_type,
		 class C_array_type		   = int_vector<>
	    >
class succinct_byte_alphabet_strategy;

//! A simple space greedy representation for byte alphabets.
/*!
 *  \par Space consumption:
 *       At least: 2.5 kB
 *       Details:  char2comp + comp2char take  2*256 + 2*8 bytes 
 *                 m_C                   takes       257*8 bytes
 *                 m_sigma               takes           2 bytes
 */
class byte_alphabet_strategy{
	public:
		typedef int_vector<>::size_type size_type;
		typedef int_vector<8>			char2comp_type;
		typedef int_vector<8>			comp2char_type;
		typedef int_vector<64>			C_type;
		typedef uint16_t				sigma_type;
	private:
	    char2comp_type	m_char2comp; // mapping from a character into the compact alphabet
        comp2char_type	m_comp2char; // inverse mapping of m_char2comp
        C_type  		m_C; 		 // cumulative counts for the compact alphabet [0..sigma]
        sigma_type		m_sigma;     // effective size of the alphabet

		void copy(const byte_alphabet_strategy&);
	public:

		const char2comp_type& char2comp;
		const comp2char_type& comp2char;
		const C_type&		  C;
		const sigma_type&     sigma;

		//! Default constructor
		byte_alphabet_strategy();

		//! Construct from a byte-stream
		/*!
		 *  \param text_buf	Byte stream.
		 *  \param len		Length of the byte stream. 
		 */
		template<class size_type_class>
		byte_alphabet_strategy(int_vector_file_buffer<8, size_type_class> &text_buf, size_type_class len): 
							   char2comp(m_char2comp), comp2char(m_comp2char), C(m_C), sigma(m_sigma)
		{
			m_sigma = 0;
    		text_buf.reset();
   			if (0 == text_buf.int_vector_size)
        		return;
			// initialize vectors 
			util::assign(m_C	    , int_vector<64>(257, 0));
			util::assign(m_char2comp, int_vector<8>(256,0));
    		util::assign(m_comp2char, int_vector<8>(256,0));
			// count occurrences of each symbol 
     		for (size_type i=0, r_sum=0, r = text_buf.load_next_block(); i < len;) {
        		for (; i < r_sum+r; ++i) {
            		++m_C[text_buf[i-r_sum]];
        		}
        		r_sum += r; r = text_buf.load_next_block();
    		}
    		assert(1 == m_C[0]); // null-byte should occur exactly once
    		m_sigma = 0;
			for (int i=0; i<256; ++i)
				if (m_C[i]) {
					m_char2comp[i] 	 	= m_sigma;
					m_comp2char[sigma]  = i; 
					m_C[m_sigma]		= m_C[i];
					++m_sigma;
				}
			m_comp2char.resize(m_sigma);
			m_C.resize(m_sigma+1);
			for (int i=(int)m_sigma; i > 0; --i) m_C[i] = m_C[i-1]; 
			m_C[0] = 0;
			for (int i=1; i <= (int)m_sigma; ++i) m_C[i] += m_C[i-1];
			assert(C[sigma]==len);
		}

		//! Copy constructor
		byte_alphabet_strategy(const byte_alphabet_strategy&);

		byte_alphabet_strategy& operator=(const byte_alphabet_strategy&);

		//! Swap operator
        void swap(byte_alphabet_strategy&);

		//! Serialize method
        size_type serialize(std::ostream& out, structure_tree_node* v=NULL, std::string name="")const;

		//! Load method
        void load(std::istream& in);
};


//! A space-efficient representation for byte alphabets.
/*!
 */
//template<class bit_vector_type, class rank_support_type, class select_support_type, class C_array_type>
//class succinct_byte_alphabet_strategy{
//	public:
//		class char2comp_wrapper;
//		class comp2char_wrapper;
//		
//
//		typedef int_vector<>::size_type size_type;
//		typedef char2comp_wrapper		char2comp_type;
//		typedef comp2char_wrapper		comp2char_type;
//		typedef C_array_type			C_type;
//		typedef uint16_t				sigma_type;
//	private:
//		bit_vector_type 	m_char;        // `m_char[i]` indicates if character with code i is present or not
//	    rank_support_type	m_char_rank;   // rank data structure for `m_char` to answer char2comp
//		select_support_type m_char_select; // select data structure for `m_char` to answer comp2char
//        C_type  			m_C; 		   // cumulative counts for the compact alphabet [0..sigma]
//        sigma_type			m_sigma;       // effective size of the alphabet
//
//		void copy(const succinct_byte_alphabet_strategy&);
//	public:
//
//		class char2comp_wrapper{
//			succinct_byte_alphabet_strategy *m_;
//			char2comp_wrapper()
//		}
//
//		class comp2char_wrapper{
//			
//		}
//
//		const char2comp_type& char2comp;
//		const comp2char_type& comp2char;
//		const C_type&		  C;
//		const sigma_type&     sigma;
//
//		//! Default constructor
//		succinct_byte_alphabet_strategy(){
//			m_sigma = 0;
//		}
//
//		//! Construct from a byte-stream
//		/*!
//		 *  \param text_buf	Byte stream.
//		 *  \param len		Length of the byte stream. 
//		 */
//		template<class size_type_class>
//		succinct_byte_alphabet_strategy(int_vector_file_buffer<8, size_type_class> &text_buf, size_type_class len): 
//							           char2comp(this), comp2char(this), C(m_C), sigma(m_sigma)
//		{
//			m_sigma = 0;
//    		text_buf.reset();
//   			if (0 == text_buf.int_vector_size)
//        		return;
//			// initialize vectors 
//			util::assign(m_C   , int_vector<64>(257, 0));
//			util::assign(m_char, bit_vector(256));
//			// count occurrences of each symbol 
//     		for (size_type i=0, r_sum=0, r = text_buf.load_next_block(); i < len;) {
//        		for (; i < r_sum+r; ++i) {
//            		++m_C[text_buf[i-r_sum]];
//        		}
//        		r_sum += r; r = text_buf.load_next_block();
//    		}
//    		assert(1 == m_C[0]); // null-byte should occur exactly once
//    		m_sigma = 0;
//			for (int i=0; i<256; ++i)
//				if (m_C[i]) {
//					m_char[i] = 1;			// mark occurring character
//					m_C[m_sigma] = m_C[i];  // compactify m_C
//					++m_sigma;
//				}
//			m_C.resize(m_sigma+1);
//			for (int i=(int)m_sigma; i > 0; --i) m_C[i] = m_C[i-1]; 
//			m_C[0] = 0;
//			for (int i=1; i <= (int)m_sigma; ++i) m_C[i] += m_C[i-1];
//			assert(C[sigma]==len);
//
//			util::init_support(m_char_rank, &m_rank);	
//			util::init_support(m_char_select, &m_rank);	
//		}
//
//		//! Copy constructor
//		succinct_byte_alphabet_strategy(const succinct_byte_alphabet_strategy&);
//
//		succinct_byte_alphabet_strategy& operator=(const succinct_byte_alphabet_strategy&);
//
//		//! Swap operator
//        void swap(succinct_byte_alphabet_strategy&);
//
//		//! Serialize method
//        size_type serialize(std::ostream& out, structure_tree_node* v=NULL, std::string name="")const;
//
//		//! Load method
//        void load(std::istream& in);
//};
//
} // end namespace sdsl

#endif