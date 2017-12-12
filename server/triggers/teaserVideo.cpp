

#include "../serverCalls.h"
#include "../map.h"



static void customInit() {
    ClothingSet clothing = getEmptyClothingSet();
    
    clothing.hat = getObject( 199 );
    clothing.tunic = getObject( 201 );
    clothing.bottom = getObject( 200 );
    clothing.frontShoe = getObject( 203 );
    clothing.backShoe = getObject( 203 );
    
    PlayerMapping narrator = {
        "test@test.com",
        19,
        20,
        { -30, -10 },
        0, clothing };
    
    playerMap.push_back( narrator );
    }




static LiveDummySocket firstBaby;





static void customTrigger( int inTriggerNumber ) {
    
    //GridPos offset = { 0, 0 };
    ClothingSet clothing = getEmptyClothingSet();

    
    // increment this as we check for matches
    // thus, we can rearrange trigger order below
    // without having to update all the trigger numbers
    int t = 0;
    
    
    if( inTriggerNumber == t++ ) {
        
        GridPos startPos = { -10, -10 };
        
        
        
        firstBaby = newDummyPlayer( "dummy2@test.com", 19, 0,
                                    startPos,
                                    0,
                                    clothing );

        // stick a loincloth to right so we can put it on baby
        setMapObject( 0, -10, 200 );
        }
    else if( inTriggerNumber == t++ ) {
        killPlayer( "test@test.com" );
        }
    else if( inTriggerNumber == t++ ) {        
        // baby walks around at end
        addToMove( 1, 0 );
        addToMove( 2, 0 );
        addToMove( 3, 0 );
        
        sendDummyMove( &firstBaby, finishMove() );
        }
    else if( inTriggerNumber == t++ ) {        
        // baby walks around at end
        addToMove( -1, 0 );
        addToMove( -2, 0 );
        
        sendDummyMove( &firstBaby, finishMove() );
        }
    else if( inTriggerNumber == t++ ) {        
        // kill baby
        
        dummySockets.deleteElementEqualTo( firstBaby.sock );

        delete firstBaby.sock;
        }
    
    }

