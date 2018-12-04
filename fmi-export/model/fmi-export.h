/* -*- Mode:C++; c-file-style:"bsd"; -*- */
#ifndef FMIPPEX_H
#define FMIPPEX_H


namespace ns3 {


#define CREATE_NS3_FMU_BACKEND( BACKENDTYPE ) \
int main( int argc, const char* argv[] ) { \
	BACKENDTYPE backend; \
	if ( 0 != backend.initializeBase( argc, argv ) ) { return -1; } \
	while ( true == backend.readyToLoop() ) { if ( 0 != backend.doStepBase() ) return -1; } \
	return 0; }


}


#endif // FMIPPEX_H

