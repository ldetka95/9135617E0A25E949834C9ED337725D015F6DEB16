
    /// CUSTOM FUNCTIONS FOR DCA
    /*
        ---
        Dynamic Copy Allocation ( DCA ) is something new in PSE2, and can be really useful.
        Because script lines ( PSE2_Line ) uses not direct data, but references ( pointers ) telling where the data is,
        they can store anything in one way, and doesn't requires any templates hell. But, the data lying under
        reference may change during the program. It's obvious then to copy required data to new memory placement.
        The functions below are doing it itself, so you don't need to worry about your data loss/overwrite: PSE2 will maintain it!
        ---
    */

    /// Update 14.01.2016
    /*
        ---
        These functions are @Discouraged from now, but it's still free to use them.
        The reason is, I'm afraid of any potential, and totally not required overuse of them - so, making trashcan of memory.
        They are useful in PSE2, and could be in some cases, but mostly - it's just @Discouraged.
        ---
    */

template < class DCA_TYPE > /// @Discouraged
    DCA_TYPE* PSE2_DCA( DCA_TYPE ___value ) {
        DCA_TYPE* ___pointer = ( DCA_TYPE* )( malloc( sizeof( DCA_TYPE ) ) );
        ( *___pointer ) = ___value;
        return ___pointer;
    }

template < class DCA_TYPE > /// @Discouraged
    void PSE2_REMDCA( DCA_TYPE* ___pointer ) {
        if ( ___pointer ) {
            free( ___pointer );
        }
    }


