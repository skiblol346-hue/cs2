#pragma once
#include <Core/Utils/Math/Math.hpp>


namespace EASING
{
	__forceinline double InSine( const double t )
	{
		return M_SIN( 1.5707963 * t );
	}

	__forceinline double OutSine( double t )
	{
		return 1 + M_SIN( 1.5707963 * ( --t ) );
	}

	__forceinline double InOutSine( const double t )
	{
		return 0.5 * ( 1 + M_SIN( 3.1415926 * ( t - 0.5 ) ) );
	}

	__forceinline double InQuad( const double t )
	{
		return t * t;
	}

	__forceinline double OutQuad( const double t )
	{
		return t * ( 2 - t );
	}

	__forceinline double InOutQuad( const double t )
	{
		return t < 0.5 ? 2 * t * t : t * ( 4 - 2 * t ) - 1;
	}

	__forceinline double InCubic( const double t )
	{
		return t * t * t;
	}

	__forceinline double OutCubic( double t )
	{
		return 1 + ( --t ) * t * t;
	}

	__forceinline double InOutCubic( double t )
	{
		return t < 0.5 ? 4 * t * t * t : 1 + ( --t ) * ( 2 * ( --t ) ) * ( 2 * t );
	}

	__forceinline double InQuart( double t )
	{
		t *= t;
		return t * t;
	}

	__forceinline double OutQuart( double t )
	{
		t = ( --t ) * t;
		return 1 - t * t;
	}

	__forceinline double InOutQuart( double t )
	{
		if ( t < 0.5 )
		{
			t *= t;
			return 8 * t * t;
		}
		else
		{
			t = ( --t ) * t;
			return 1 - 8 * t * t;
		}
	}

	__forceinline double InQuint( const double t )
	{
		const double t2 = t * t;
		return t * t2 * t2;
	}

	__forceinline double OutQuint( double t )
	{
		const double t2 = ( --t ) * t;
		return 1 + t * t2 * t2;
	}

	__forceinline double InOutQuint( double t )
	{
		double t2;
		if ( t < 0.5 )
		{
			t2 = t * t;
			return 16 * t * t2 * t2;
		}
		else
		{
			t2 = ( --t ) * t;
			return 1 + 16 * t * t2 * t2;
		}
	}

	__forceinline double InExpo( const double t )
	{
		return ( M_POW( 2.0, 8 * t ) - 1 ) / 255;
	}

	__forceinline double OutExpo( const double t )
	{
		return 1 - M_POW( 2.0, -8 * t );
	}

	__forceinline double InOutExpo( const double t )
	{
		if ( t < 0.5 )
		{
			return ( M_POW( 2.0, 16 * t ) - 1 ) / 510;
		}
		else
		{
			return 1 - 0.5 * M_POW( 2.0, -16 * ( t - 0.5 ) );
		}
	}

	__forceinline double InCirc( const double t )
	{
		return 1 - M_SQRT( 1 - t );
	}

	__forceinline double OutCirc( const double t )
	{
		return M_SQRT( t );
	}

	__forceinline double InOutCirc( const double t )
	{
		if ( t < 0.5 )
		{
			return ( 1 - M_SQRT( 1 - 2 * t ) ) * 0.5;
		}
		else
		{
			return ( 1 + M_SQRT( 2 * t - 1 ) ) * 0.5;
		}
	}

	__forceinline double InBack( const double t )
	{
		return t * t * ( 2.70158 * t - 1.70158 );
	}

	__forceinline double OutBack( double t )
	{
		return 1 + ( --t ) * t * ( 2.70158 * t + 1.70158 );
	}

	__forceinline double InOutBack( double t )
	{
		if ( t < 0.5 )
		{
			return t * t * ( 7 * t - 2.5 ) * 2;
		}
		else
		{
			return 1 + ( --t ) * t * 2 * ( 7 * t + 2.5 );
		}
	}

	__forceinline double InElastic( const double t )
	{
		const double t2 = t * t;
		return t2 * t2 * M_SIN( t * MATH::_PI * 4.5 );
	}

	__forceinline double OutElastic( const double t )
	{
		const double t2 = ( t - 1 ) * ( t - 1 );
		return 1 - t2 * t2 * M_COS( t * MATH::_PI * 4.5 );
	}

	__forceinline double InOutElastic( const double t )
	{
		double t2;
		if ( t < 0.45 )
		{
			t2 = t * t;
			return 8 * t2 * t2 * M_SIN( t * MATH::_PI * 9 );
		}
		else if ( t < 0.55 )
		{
			return 0.5 + 0.75 * M_SIN( t * MATH::_PI * 4 );
		}
		else
		{
			t2 = ( t - 1 ) * ( t - 1 );
			return 1 - 8 * t2 * t2 * M_SIN( t * MATH::_PI * 9 );
		}
	}

	__forceinline double InBounce( const double t )
	{
		return M_POW( 2.0, 6 * ( t - 1 ) ) * M_ABS( M_SIN( t * MATH::_PI * 3.5 ) );
	}

	__forceinline double OutBounce( const double t )
	{
		return 1 - M_POW( 2.0, -6 * t ) * M_ABS( M_COS( t * MATH::_PI * 3.5 ) );
	}

	__forceinline double InOutBounce( const double t )
	{
		if ( t < 0.5 )
		{
			return 8 * M_POW( 2.0, 8 * ( t - 1 ) ) * M_ABS( M_SIN( t * MATH::_PI * 7 ) );
		}
		else
		{
			return 1 - 8 * M_POW( 2.0, -8 * t ) * M_ABS( M_SIN( t * MATH::_PI * 7 ) );
		}
	}
}
