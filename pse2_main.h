#pragma once

#include "pse2_alloctypes.h"





/*
    ---
    PSE2_Argument
    ---
    ---
*/

class PSE2_Argument {
    public:
        /// Constructors
    PSE2_Argument() {
        argArray = NULL;
        argNumber = 0;
    }
    PSE2_Argument( void* new_ArgArray[], int new_ArgNumber ) { /// @Discouraged unless you really know what are you doing
        /// This function does not use DCA
        if ( new_ArgNumber > 0 ) {
            argNumber = new_ArgNumber;
            argArray = ( void* (*)[] )( malloc( new_ArgNumber * sizeof( void* ) ) );
            if ( !argArray ) {
                argNumber = 0;
            } else {
                for ( int i = 0; i < new_ArgNumber; i++ ) {
                    ( *argArray )[ i ] = new_ArgArray[ i ];
                }
                //printf( "Done!\n" );
            }
        } else {
            argNumber = 0;
            argArray = NULL;
        }
    }
        /// Destructor
    ~PSE2_Argument() {
        if ( argArray ) {
            //if ( argArray )
            free( argArray );
            //printf( "Cleaned!\n" );
        }
    }
        /// Get and Set arguments
    void* GetArgument( int index ) {
        if ( ( index >= 0 ) && ( index < argNumber ) ) {
            return ( *argArray )[ index ];
        }
        return NULL;
    }
    void* SetArgument( int index, void* new_Value ) {
        if ( ( index >= 0 ) && ( index < argNumber ) ) {
            void* ret_Ptr = ( *argArray )[ index ];
            ( *argArray )[ index ] = new_Value;
            return ret_Ptr;
        }
        return NULL;
    }
        /// Add argument
    template < class Targument >
        void AppendArgument( Targument value ) {
            Resize( argNumber + 1 );
            Targument* arg_Ptr = PSE2_DCA( value );
            ( *argArray )[ argNumber - 1 ] = arg_Ptr;
        }
        /// Get arguments quantity
    int Quantity() {
        return argNumber;
    }
        /// Get an argument array pointer
    void** Arguments() {
        return ( void** )( argArray );
    }
    private:
    void* ( *argArray )[];
    int argNumber;
    void Resize( int new_Size ) {
        if ( new_Size > 0 ) {
            argArray = ( void* (*)[] )( realloc( argArray, new_Size * sizeof( void* ) ) );
            argNumber = new_Size;
        } else {
            if ( argArray ) {
                free( argArray );
                argArray = NULL;
            }
            argNumber = 0;
        }
    }
};



/*
    ---
    PSE2_Interpreter
    ---
    Interpreter is used to interpretate the incoming calls from PSE_Line.
    Prototype for call function is:
        void* foo( void* caller, PSE2_Argument* args );
    ---
*/

class PSE2_Interpreter {
    public:
        /// Constructor
        /*
            ---
            There are several types of constuctors:
                Constructor()
                     - default, no space for calls is given, available for modyfing.
                Constructor( int n )
                     - prepares a space for n calls, with lowest index at 0 and highest at ( n - 1 ).
            ---
        */
    PSE2_Interpreter() {
        callArray = NULL;
        instructionNumber = 0;
    }
    PSE2_Interpreter( int new_instructionNumber ) {
        callArray = NULL;
        instructionNumber = 0;
        Resize( new_instructionNumber );
    }
        /// Destructor
        /*
            ---
            At destruction, Flush() is called and everything is safely removed from the system.
            ---
        */
    ~PSE2_Interpreter() {
        Flush();
    }
        /// ADDENTRY - register an entry for call function, or bind function to given code
        /*
            ---
            Takes a code to bind as a parameter, and pointer to function of PSE2 call prototype.
            See PSE2_Interpreter description for more info.
            Function returns 0 on success or -1 if there is no space to bind the function to specific code.
            ---
        */
    int AddEntry( int code, void* ( *entryFunc )( void*, PSE2_Argument* ) ) {
        if ( ( code >= 0 ) && ( code < instructionNumber ) ) {
            ( *callArray )[ code ] = ( void* )( entryFunc );
            return 0;
        }
        return -1;
    }
        /// APPENDENTRY - register an entry for call function, or bind function to given code
        /*
            ---
            Takes a code to bind as a parameter, and pointer to function of PSE2 call prototype.
            See PSE2_Interpreter description for more info.
            PSE2_Interpreter automatically resizes the binded calls array, if there is no space to bind the function.
            Function always returns 0. Any fails comes from the system, i.e. if call array is too large.
            ---
        */
    int AppendEntry( int code, void* ( *entryFunc )( void*, void*, int ) ) {
        if ( ( code < 0 ) || ( code >= instructionNumber ) ) {
            Resize( code + 1 );
        }
        ( *callArray )[ code ] = ( void* )( entryFunc );
        return 0;
    }
        /// FLUSH - remove all registered calls from system and delete the calls array
        /*
            ---
            No parameters are taken.
            This functions also destroys the calls array, so be careful when you use this.
            It is being called on destruction state, so there is usually no need to do this manually.
            ---
        */
    void Flush() {
        if ( callArray ) {
            free( callArray );
            callArray = NULL;
        }
        instructionNumber = 0;
    }
        /// CALL - calls instruction with given parameters
        /*
            ---
            Safely calls linked entry for given code and pass arguments to it.
            ---
        */
    void* Call( int code, void* caller, PSE2_Argument* args ) {
        if ( ( code >= 0 ) && ( code < instructionNumber ) ) {
            void* ( *func )( void*, PSE2_Argument* ) = ( void* (*)( void*, PSE2_Argument* ) )( *callArray )[ code ];
            if ( func ) {
                //printf( "Called function: %d\n", code );
                return ( *func )( caller, args );
            } else {
                //printf( "UNRECOGNIZED FUNCTION: %d\n", code );
            }
        }
        return NULL;
    }
    private:
    void* ( *callArray )[];
    int instructionNumber;
    void Resize( int new_instructionNumber ) {
        if ( new_instructionNumber > 0 ) {
            callArray = ( void* (*)[] )( realloc( callArray, new_instructionNumber * sizeof( void* ) ) );
            for ( int i = instructionNumber; i < new_instructionNumber; i++ ) {
                ( *callArray )[ i ] = NULL;
            }
        } else {
            Flush();
        }
        instructionNumber = new_instructionNumber;
    }
};



/*
    ---
    PSE2_Line
    ---
    ---
*/

class PSE2_Line {
    public:
        /// Constructors
        /*
            ---
            There are several types of constuctors:
                Constructor()
                     - default, no elements nor interpreter are included, available for modyfing.
                Constructor( int n )
                     - prepares a space for n elements, and no interpreter is linked.
                Constructor( PSE2_Interpreter* i )
                     - no elements are included, and interpreter i is linked.
                Constructor( PSE2_Interpreter* i, int n )
                     - prepares a space for n elements, and interpreter i is linked.
            ---
        */
    PSE2_Line() {
        Construct( 0, NULL );
    }
    PSE2_Line( int new_eNumber ) {
        Construct( new_eNumber, NULL );
    }
    PSE2_Line( PSE2_Interpreter* new_Interpreter ) {
        Construct( 0, new_Interpreter );
    }
    PSE2_Line( PSE2_Interpreter* new_Interpreter, int new_eNumber ) {
        Construct( new_eNumber, new_Interpreter );
    }
        /// Destructor
        /*
            ---
            At destruction, Flush() is called and everything is safely removed from the system.
            ---
        */
    ~PSE2_Line() {
        Flush();
    }
        /// FLUSH - remove all calls from line
        /*
            ---
            No parameters are taken.
            Destruction process is a safe function and can be called at any state.
            ---
        */
    void Flush() {
        if ( elementArray ) {
            for ( int i = 0; i < elementNumber; i++ ) {
                if ( ( *elementArray )[ i ] ) {
                    delete ( *elementArray )[ i ];
                }
            }
            free( elementArray );
            elementArray = NULL;
        }
        elementNumber = 0;
    }
        /// HARDFLUSH - remove all calls from line, and allocated memory - UNUSED RIGHT NOW
        /*
            ---
            No parameters are taken.
            Does the same thing as Flush(), but should be called ONLY if using DCA.
            ---
        */
    /* void HardFlush() {
        if ( elementArray ) {
            for ( int i = 0; i < elementNumber; i++ ) {
                delete ( *elementArray )[ i ];
            }
            free( elementArray );
            elementArray = NULL;
        }
        elementNumber = 0;
    } */
        /// INTERPRETER method
        /*
            ---
            Links an interpreter to process calls array.
            Can be changed at any time, but it is a good practice not to do this.
            If an input parameter is NULL, interpreter is not changed.
            Returns an old linked interpreter, or current, when passing NULL as argument.
            ---
        */
    PSE2_Interpreter* Interpreter( PSE2_Interpreter* new_Interpreter ) {
        PSE2_Interpreter* old_Interpreter;
        if ( new_Interpreter ) {
            interpreter = new_Interpreter;
        }
        return old_Interpreter;
    }
        /// PROCESS
        /*
            ---
            ---
        */
    int Process() {
        if ( ( elementCursor < elementNumber ) && ( interpreter ) ) {
            PSE2_Element* processed = ( *elementArray )[ elementCursor ];
            int callCode = processed -> code;
            PSE2_Argument* callArgs = processed -> args;
            interpreter -> Call( callCode, ( void* )( this ), callArgs );
            return 0;
        }
        return -1;
    }
    void NextInstruction() {
        elementCursor++;
    }
    void ResetCursor() {
        elementCursor = 0;
    }
    void JumpInstruction( int new_Cursor ) {
        elementCursor = new_Cursor;
    }
    int CurrentInstruction() {
        return elementCursor;
    }
        /// APPEND
        /*
            ---
            ---
        */
    void Append( int code, PSE2_Argument* args ) {
        Resize( elementNumber + 1 );
        PSE2_Element* element = new PSE2_Element( code, args );
        ( *elementArray )[ elementNumber - 1 ] = element;
    }
        /// SIZE - take actual size of the script line
        /*
            ---
            No parameters are taken.
            Safe as it can be.
            ---
        */
    int Size() {
        return elementNumber;
    }
        /// Private data
    private:
        /// Element class
        /*
            ---
            This class represents one object, or one element in calls array, or one instruction to process.
            Uses default PSE2_Interpreter to interpretate the instruction.
            This is a private class, so it cannot be used alone nor outside the system.
            ---
        */
    class PSE2_Element {
        public:
        PSE2_Element( int new_Code, PSE2_Argument* new_Args ) {
            code = new_Code;
            args = new PSE2_Argument( new_Args -> Arguments(), new_Args -> Quantity() );
        }
        ~PSE2_Element() {
            delete args;
        }
        int code;
        PSE2_Argument* args;
    };
        /// Element dynamic array
        /*
            ---
            Element array is full dynamic and cannot fail itself.
            The only fails/throws/errors comes from the system,
            usually if memory block is too large ( i.e. 5 million of instructions ).
            ---
        */
    PSE2_Element* ( *elementArray )[];
    int elementNumber;
    int elementCursor;
    PSE2_Interpreter* interpreter;
    /// Private methods
        /// CONSTRUCT
        /*
            ---
            Standard method for construction.
            Private for internal use.
            ---
        */
    void Construct( int eNumber, PSE2_Interpreter* new_Interpreter ) {
        elementArray = NULL;
        elementNumber = 0;
        elementCursor = 0;
        Resize( eNumber );
        interpreter = new_Interpreter;
    }
    void Resize( int new_eNumber ) {
        if ( new_eNumber > 0 ) {
            elementArray = ( PSE2_Element* (*)[] )( realloc( elementArray, new_eNumber * sizeof( PSE2_Element* ) ) );
            for ( int i = elementNumber; i < new_eNumber; i++ ) {
                ( *elementArray )[ i ] = NULL;
            }
            elementNumber = new_eNumber;
        } else {
            Flush();
        }
    }
};


// Author: £ukasz Detka

