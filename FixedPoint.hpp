//
//  FixedPoint.hpp
//  Teensy
//
//  Created by Colin on 4/21/17.
//  Copyright © 2017 Colin Duffy. All rights reserved.
//

#ifndef FixedPoint_hpp
#define FixedPoint_hpp

#include "Arduino.h"

// computes limit(val, 2**bits)
static inline int16_t saturate16_i(int32_t val) {
#if defined(KINETISK)
    int16_t out;
    int32_t tmp;
    asm volatile("ssat %0, %1, %2" : "=r" (tmp) : "I" (16), "r" (val) );
    out = (int16_t) (tmp & 0xffff); // not sure if the & 0xffff is necessary. test.
    return out;
#elif defined(KINETISL)
    return 0; // TODO....
#endif
}

template <class BaseType, size_t num_frac_bit>
class fixed_point {
public:
    
    fixed_point( void ) : data ( 0 ) { }
    
    fixed_point( double d ) : data ( 0 ) {
        *this = d;// calls operator=
    }
    
    // ======================================================================================
    fixed_point & operator = ( double d ) {
        //data = static_cast<BaseType>( d * factor );
        data = round( d * ( 1 << num_frac_bit ) );
        //data = saturate16_i( d * ( 1 << num_frac_bit ) );
        return *this;
    }
    
    fixed_point & operator = ( BaseType d ) {
        data = d;
        return *this;
    }
    
    
    /*template <class Base>
    fixed_point & operator = ( const Base &rhs ) {
        //Serial.println("====");
        //fixed_point<BaseType, num_frac_bit> fp = rhs;
        //Serial.printf("==== | sizeof: %i | FD: %i \n", sizeof(BaseType), num_frac_bit);
        this->data = rhs.data;
        return *this;
    }*/
    
    
    fixed_point & operator = ( const fixed_point &rhs ) {
        //Serial.printf("=== | rhs: %i | this: %i\n", rhs.data, this->data);
        //fixed_point<BaseType, num_frac_bit> fp = *this;
        this->data = rhs.data;
        return *this;
        //return rhs;
    }


    operator BaseType ( void ) const {
        //Serial.printf("this data: %i | data: %i\n", this->data, data);
        return data;
    }
    
    // ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    fixed_point operator + ( const fixed_point &rhs ) const {
        fixed_point<BaseType, num_frac_bit> fp = *this;
        fp.data += rhs.data;
        //Serial.println("+");
        return fp;
    }
    
    fixed_point operator + ( const double rhs ) const {
        //Serial.printf("++ | rhs: %i | data: %i\n", rhs, this->data);
        fixed_point<BaseType, num_frac_bit> fp = *this;
        fp.data += static_cast<BaseType>( rhs * factor );
        return fp;
    }
    
    friend fixed_point operator + ( const double lhs, const fixed_point &rhs ) {
        fixed_point<BaseType, num_frac_bit> fp = rhs;
        BaseType d = static_cast<BaseType>( lhs * rhs.factor );
        fp.data = d + rhs.data;
        return fp;
    }
    
    // ---------------------------------------------------------------------------------------
    fixed_point operator - ( const fixed_point &rhs ) const {
        fixed_point<BaseType, num_frac_bit> fp = *this;
        fp.data -= rhs.data;
        //Serial.println("-");
        return fp;
    }
    
    fixed_point operator - ( const double rhs ) const {
        //Serial.printf("-- | rhs: %i | data: %i\n", rhs, this->data);
        fixed_point<BaseType, num_frac_bit> fp = *this;
        fp.data -= static_cast<BaseType>( rhs * factor );
        return fp;
    }
    
    friend fixed_point operator - ( const double lhs, const fixed_point &rhs ) {
        fixed_point<BaseType, num_frac_bit> fp = rhs;
        BaseType d = static_cast<BaseType>( lhs * rhs.factor );
        fp.data = d - rhs.data;
        return fp;
    }

    // ***************************************************************************************
    fixed_point operator * ( const fixed_point &rhs ) const {
        fixed_point<BaseType, num_frac_bit> fp;// = *this;
        uint32_t tmp = ( ( int64_t )this->data * rhs.data ) >> num_frac_bit;
        //uint32_t r = tmp + ( ( tmp & 1 << ( num_frac_bit - 1 ) ) << 1 );
        //r >>= num_frac_bit;
        fp.data = tmp;
        //Serial.printf("tmp: %f\n", fp.baseToFloat());
        //Serial.printf("fp data: %f\n", fp.baseToFloat());
        return fp;
    }
    
    fixed_point operator * ( const double rhs ) const {
        fixed_point<BaseType, num_frac_bit> fp;// = *this;
        uint32_t tmp = ( ( int64_t )this->data * static_cast<BaseType>( rhs * factor ) ) >> num_frac_bit;
        //uint32_t r = tmp + ( ( tmp & 1 << ( num_frac_bit - 1 ) ) << 1 );
        //r >>= num_frac_bit;
        fp.data = tmp;
        return fp;
    }
    
    friend fixed_point operator * ( double lhs, const fixed_point &rhs ) {
        fixed_point<BaseType, num_frac_bit> fp;// = rhs;
        uint32_t tmp = ( ( int64_t )rhs.data * static_cast<BaseType>( lhs * rhs.factor ) ) >> num_frac_bit;
        //uint32_t r = tmp + ( ( tmp & 1 << ( num_frac_bit - 1 ) ) << 1 );
        //r >>= num_frac_bit;
        fp.data = tmp;
        return fp;
    }
    
    // ////////////////////////////////////////////////////////////////////////////////////////
    fixed_point operator / ( const fixed_point &rhs ) const {
        fixed_point<BaseType, num_frac_bit> fp = *this;
        uint32_t r = ( ( this->data << num_frac_bit ) / rhs.data );
        fp.data = r;
        return fp;
    }
    
    fixed_point operator / ( const double rhs ) const {
        fixed_point<BaseType, num_frac_bit> fp = *this;
        uint32_t r = ( ( this->data << num_frac_bit ) / static_cast<BaseType>( rhs * factor ) );
        fp.data = r;
        return fp;
    }
    
    friend fixed_point operator / ( double lhs, const fixed_point &rhs ) {
        fixed_point<BaseType, num_frac_bit> fp = rhs;
        BaseType fp_lhs = static_cast<BaseType>( lhs * rhs.factor );
        BaseType fp_div = ( ( double )( fp_lhs << num_frac_bit ) / ( double )( rhs.data ) );
        fp.data = fp_div;
        return fp;
    }

    // ########################################################################################
     // transcendental functions
    fixed_point sqrtfp( fixed_point fp_in ) {
        register BaseType root, remHi, remLo, count;
        /* BaseType << 3 integer bits, num_frac_bit fractional bits */
        fixed_point p;
        root = 0;       /* Clear root */
        remHi = 0;      /* Clear high part of partial remainder */
        remLo = fp_in;  /* Get argument into low part of partial remainder */
        count = num_frac_bit; /* Load loop counter */
        BaseType tt;
        do {
            remHi = ( remHi << 2 ) | ( remLo >> 26 );
            remLo <<= 6;//interger_shift; /* get 2 bits of arg */
            root <<= 1; /* Get ready for the next bit in the root */
            BaseType testDiv = ( root << 1 ) + 1;
            Serial.printf("remHi1: %10u | root: %10u | tesDiv: %10u | ", remHi, root, testDiv);
            if ( remHi >= testDiv ) {
                remHi -= testDiv;
                root++;
            }
            tt = testDiv;
            Serial.printf("root: %10u | remHi: %10u | remLo: %10u | count: %u\n", root, remHi, remLo, count);
        } while ( count-- != 0 );
        p = tt;//root;
        return p;
    }
    // ########################################################################################
    BaseType raw_data( void ) const { return data; }
    
    float baseToFloat( void ) {
        double fp = data;
        fp = fp / double( factor ); // multiplication by a constant
        return fp;
    }
protected:

private:
    const BaseType factor = 1 << num_frac_bit;
    const uint8_t  interger_shift = sizeof( BaseType ) << 1;
    BaseType data;
};
#endif /* FixedPoint_hpp */
