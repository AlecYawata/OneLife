#include "TextField.h"

#include <string.h>
#include <mbstring.h>

#include "minorGems/game/game.h"
#include "minorGems/game/gameGraphics.h"
#include "minorGems/game/drawUtils.h"
#include "minorGems/util/log/AppLog.h"
#include "minorGems/util/stringUtils.h"
#include "minorGems/util/SimpleVector.h"
#include "minorGems/graphics/openGL/KeyboardHandlerGL.h"



// start:  none focused
TextField *TextField::sFocusedTextField = NULL;

extern double frameRateFactor;

int TextField::sDeleteFirstDelaySteps = 30 / frameRateFactor;
int TextField::sDeleteNextDelaySteps = 2 / frameRateFactor;




TextField::TextField( Font *inDisplayFont, 
                      double inX, double inY, int inCharsWide,
                      char inForceCaps,
                      const char *inLabelText,
                      const char *inAllowedChars,
                      const char *inForbiddenChars,
                      bool allowHiragana )
        : PageComponent( inX, inY ),
          mActive( true ), 
          mContentsHidden( false ),
          mHiddenSprite( loadSprite( "hiddenFieldTexture.tga", false ) ),
          mFont( inDisplayFont ), 
          mCharsWide( inCharsWide ),
          mMaxLength( -1 ),
          mFireOnAnyChange( false ),
          mFireOnLeave( false ),
          mForceCaps( inForceCaps ),
          mLabelText( NULL ),
          mAllowedChars( NULL ), mForbiddenChars( NULL ),
          mFocused( false ), mText( new char[1] ),
          mTextLen( 0 ),
          mCursorPosition( 0 ),
          mIgnoreArrowKeys( false ),
          mIgnoreMouse( false ),
          mDrawnText( NULL ),
          mCursorDrawPosition( 0 ),
          mHoldDeleteSteps( -1 ), mFirstDeleteRepeatDone( false ),
          mLabelOnRight( false ),
          mLabelOnTop( false ),
          mSelectionStart( -1 ),
          mSelectionEnd( -1 ),
          mShiftPlusArrowsCanSelect( false ),
          mAllowHiragana( allowHiragana ),
          mCursorFlashSteps( 0 ) {
    
    if( inLabelText != NULL ) {
        mLabelText = stringDuplicate( inLabelText );
        }
    
    if( inAllowedChars != NULL ) {
        mAllowedChars = stringDuplicate( inAllowedChars );
        }
    if( inForbiddenChars != NULL ) {
        mForbiddenChars = stringDuplicate( inForbiddenChars );
        }

    clearArrowRepeat();
        

    mCharWidth = mFont->getFontHeight();

    mBorderWide = mCharWidth * 0.25;

    mHigh = mFont->getFontHeight() + 2 * mBorderWide;

    char *fullString = new char[ mCharsWide + 1 ];

    unsigned char widestChar = 0;
    double width = 0;

    for( int c=32; c<256; c++ ) {
        unsigned char src[] = { (unsigned char)c, '\0', };
        unsigned char* pPc = processCharacter( src );
        unsigned char pc = pPc[0];
        delete [] pPc;

        if( pc != 0 ) {
            char s[2];
            s[0] = pc;
            s[1] = '\0';

            double thisWidth = mFont->measureString( s );
            
            if( thisWidth > width ) {
                width = thisWidth;
                widestChar = pc;    
                }
            }
        }
    
    


    for( int i=0; i<mCharsWide; i++ ) {
        fullString[i] = widestChar;
        }
    fullString[ mCharsWide ] = '\0';
    
    double fullStringWidth = mFont->measureString( fullString );

    delete [] fullString;

    mWide = fullStringWidth + 2 * mBorderWide;
    
    mDrawnTextX = - ( mWide / 2 - mBorderWide );

    mText[0] = '\0';
    }



TextField::~TextField() {
    if( this == sFocusedTextField ) {
        // we're focused, now nothing is focused
        sFocusedTextField = NULL;
        }

    delete [] mText;

    if( mLabelText != NULL ) {
        delete [] mLabelText;
        }

    if( mAllowedChars != NULL ) {
        delete [] mAllowedChars;
        }
    if( mForbiddenChars != NULL ) {
        delete [] mForbiddenChars;
        }

    if( mDrawnText != NULL ) {
        delete [] mDrawnText;
        }

    if( mHiddenSprite != NULL ) {
        freeSprite( mHiddenSprite );
        }
    }



void TextField::setContentsHidden( char inHidden ) {
    mContentsHidden = inHidden;
    }




void TextField::setText( const char *inText ) {
    delete [] mText;
    
    mSelectionStart = -1;
    mSelectionEnd = -1;

    // obeys same rules as typing (skip blocked characters)
    SimpleVector<char> filteredText;
    
    int length = strlen( inText );
    for( int i=0; i<length; i++ ) {
        unsigned char *pProcessedChar = processCharacter( (unsigned char*)inText + i );
        
        size_t strLength = mbclen( (char*)pProcessedChar );
        if( strLength == 1) {
            filteredText.push_back( pProcessedChar[0] );
            }
        else if( strLength == 2) {
            filteredText.push_back( pProcessedChar[0] );
            filteredText.push_back( pProcessedChar[1] );
            }
        else if( strLength == 3) {
            filteredText.push_back( pProcessedChar[0] );
            filteredText.push_back( pProcessedChar[1] );
            filteredText.push_back( pProcessedChar[2] );
            }
        delete [] pProcessedChar;
        if( strLength > 0 ) {
            i += strLength - 1;
            }
        }
    

    mText = filteredText.getElementString();
    mTextLen = mbslen( mText );
    
    mCursorPosition = strlen( mText );

    // hold-downs broken
    mHoldDeleteSteps = -1;
    mFirstDeleteRepeatDone = false;

    clearArrowRepeat();
    }



char *TextField::getText() {
    return stringDuplicate( mText );
    }



void TextField::setMaxLength( int inLimit ) {
    mMaxLength = inLimit;
    }



int TextField::getMaxLength() {
    return mMaxLength;
    }



char TextField::isAtLimit() {
    if( mMaxLength == -1 ) {
        return false;
        }
    else {
        return ( mTextLen == mMaxLength );
        }
    }
    



void TextField::setActive( char inActive ) {
    mActive = inActive;
    }



char TextField::isActive() {
    return mActive;
    }
        


void TextField::step() {

    mCursorFlashSteps ++;

    if( mHoldDeleteSteps > -1 ) {
        mHoldDeleteSteps ++;

        int stepsBetween = sDeleteFirstDelaySteps;
        
        if( mFirstDeleteRepeatDone ) {
            stepsBetween = sDeleteNextDelaySteps;
            }
        
        if( mHoldDeleteSteps > stepsBetween ) {
            // delete repeat
            mHoldDeleteSteps = 0;
            mFirstDeleteRepeatDone = true;
            
            deleteHit();
            }
        }


    for( int i=0; i<2; i++ ) {
        
        if( mHoldArrowSteps[i] > -1 ) {
            mHoldArrowSteps[i] ++;

            int stepsBetween = sDeleteFirstDelaySteps;
        
            if( mFirstArrowRepeatDone[i] ) {
                stepsBetween = sDeleteNextDelaySteps;
                }
        
            if( mHoldArrowSteps[i] > stepsBetween ) {
                // arrow repeat
                mHoldArrowSteps[i] = 0;
                mFirstArrowRepeatDone[i] = true;
            
                switch( i ) {
                    case 0:
                        leftHit();
                        break;
                    case 1:
                        rightHit();
                        break;
                    }
                }
            }
        }


    }

        
        
void TextField::draw() {
    
    if( mFocused ) {    
        setDrawColor( 1, 1, 1, 1 );
        }
    else {
        setDrawColor( 0.5, 0.5, 0.5, 1 );
        }
    

    drawRect( - mWide / 2, - mHigh / 2, 
              mWide / 2, mHigh / 2 );
    
    setDrawColor( 0.25, 0.25, 0.25, 1 );
    double pixWidth = mCharWidth / 8;


    double rectStartX = - mWide / 2 + pixWidth;
    double rectStartY = - mHigh / 2 + pixWidth;

    double rectEndX = mWide / 2 - pixWidth;
    double rectEndY = mHigh / 2 - pixWidth;

    double middleWidth = mWide - 2 * pixWidth;
    
    drawRect( rectStartX, rectStartY,
              rectEndX, rectEndY );
    
    setDrawColor( 1, 1, 1, 1 );

    if( mContentsHidden && mHiddenSprite != NULL ) {
        startAddingToStencil( false, true );

        drawRect( rectStartX, rectStartY,
                  rectEndX, rectEndY );
        startDrawingThroughStencil();
        
        doublePair pos = { 0, 0 };
        
        drawSprite( mHiddenSprite, pos );
        
        stopStencil();
        }
    


    
    if( mLabelText != NULL ) {
        TextAlignment a = alignRight;
        double xPos = -mWide/2 - mBorderWide;
        
        double yPos = 0;
        
        if( mLabelOnTop ) {
            xPos += mBorderWide + pixWidth;
            yPos = mHigh / 2 + 2 * mBorderWide;
            }

        if( mLabelOnRight ) {
            a = alignLeft;
            xPos = -xPos;
            }
        
        if( mLabelOnTop ) {
            // reverse align if on top
            if( a == alignLeft ) {
                a = alignRight;
                }
            else {
                a = alignLeft;
                }
            }
        
        doublePair labelPos = { xPos, yPos };
        
        mFont->drawString( mLabelText, labelPos, a );
        }
    
    
    if( mContentsHidden ) {
        return;
        }


    doublePair textPos = { - mWide/2 + mBorderWide, 0 };


    char tooLongFront = false;
    char tooLongBack = false;
    
    mCursorDrawPosition = mCursorPosition;


    char *textBeforeCursorBase = stringDuplicate( mText );
    char *textAfterCursorBase = stringDuplicate( mText );
    
    char *textBeforeCursor = textBeforeCursorBase;
    char *textAfterCursor = textAfterCursorBase;

    textBeforeCursor[ mCursorPosition ] = '\0';
    
    textAfterCursor = &( textAfterCursor[ mCursorPosition ] );

    if( mFont->measureString( mText ) > mWide - 2 * mBorderWide ) {
        
        if( mFont->measureString( textBeforeCursor ) > 
            mWide / 2 - mBorderWide
            &&
            mFont->measureString( textAfterCursor ) > 
            mWide / 2 - mBorderWide ) {

            // trim both ends

            while( mFont->measureString( textBeforeCursor ) > 
                   mWide / 2 - mBorderWide ) {
                
                tooLongFront = true;
                
                textBeforeCursor = &( textBeforeCursor[1] );
                
                mCursorDrawPosition --;
                }
        
            while( mFont->measureString( textAfterCursor ) > 
                   mWide / 2 - mBorderWide ) {
                
                tooLongBack = true;
                
                textAfterCursor[ strlen( textAfterCursor ) - 1 ] = '\0';
                }
            }
        else if( mFont->measureString( textBeforeCursor ) > 
                 mWide / 2 - mBorderWide ) {

            // just trim front
            char *sumText = concatonate( textBeforeCursor, textAfterCursor );
            
            while( mFont->measureString( sumText ) > 
                   mWide - 2 * mBorderWide ) {
                
                tooLongFront = true;
                
                textBeforeCursor = &( textBeforeCursor[1] );
                
                mCursorDrawPosition --;
                
                delete [] sumText;
                sumText = concatonate( textBeforeCursor, textAfterCursor );
                }
            delete [] sumText;
            }    
        else if( mFont->measureString( textAfterCursor ) > 
                 mWide / 2 - mBorderWide ) {
            
            // just trim back
            char *sumText = concatonate( textBeforeCursor, textAfterCursor );

            while( mFont->measureString( sumText ) > 
                   mWide - 2 * mBorderWide ) {
                
                tooLongBack = true;
                
                textAfterCursor[ strlen( textAfterCursor ) - 1 ] = '\0';
                delete [] sumText;
                sumText = concatonate( textBeforeCursor, textAfterCursor );
                }
            delete [] sumText;
            }
        }

    
    if( mDrawnText != NULL ) {
        delete [] mDrawnText;
        }
    
    mDrawnText = concatonate( textBeforeCursor, textAfterCursor );

    char leftAlign = true;
    char cursorCentered = false;
    doublePair centerPos = { 0, 0 };
    
    if( ! tooLongFront ) {
        mFont->drawString( mDrawnText, textPos, alignLeft );
        mDrawnTextX = textPos.x;
        }
    else if( tooLongFront && ! tooLongBack ) {
        
        leftAlign = false;

        doublePair textPos2 = { mWide/2 - mBorderWide, 0 };

        mFont->drawString( mDrawnText, textPos2, alignRight );
        mDrawnTextX = textPos2.x - mFont->measureString( mDrawnText );
        }
    else {
        // text around perfectly centered cursor
        cursorCentered = true;
        
        double beforeLength = mFont->measureString( textBeforeCursor );
        
        double xDiff = centerPos.x - ( textPos.x + beforeLength );
        
        doublePair textPos2 = textPos;
        textPos2.x += xDiff;

        mFont->drawString( mDrawnText, textPos2, alignLeft );
        mDrawnTextX = textPos2.x;
        }
    

    double shadeWidth = 4 * mCharWidth;
    
    if( shadeWidth > middleWidth / 2 ) {
        shadeWidth = middleWidth / 2;
        }

    if( tooLongFront ) {
        // draw shaded overlay over left of string
        
        double verts[] = { rectStartX, rectStartY,
                           rectStartX, rectEndY,
                           rectStartX + shadeWidth, rectEndY,
                           rectStartX + shadeWidth, rectStartY };
        float vertColors[] = { 0.25, 0.25, 0.25, 1,
                               0.25, 0.25, 0.25, 1,
                               0.25, 0.25, 0.25, 0,
                               0.25, 0.25, 0.25, 0 };

        drawQuads( 1, verts , vertColors );
        }
    if( tooLongBack ) {
        // draw shaded overlay over right of string
        
        double verts[] = { rectEndX - shadeWidth, rectStartY,
                           rectEndX - shadeWidth, rectEndY,
                           rectEndX, rectEndY,
                           rectEndX, rectStartY };
        float vertColors[] = { 0.25, 0.25, 0.25, 0,
                               0.25, 0.25, 0.25, 0,
                               0.25, 0.25, 0.25, 1,
                               0.25, 0.25, 0.25, 1 };

        drawQuads( 1, verts , vertColors );
        }
    
    if( mFocused && mCursorDrawPosition > -1 ) {            
        // make measurement to draw cursor

        char *beforeCursorText = stringDuplicate( mDrawnText );
        
        beforeCursorText[ mCursorDrawPosition ] = '\0';
        
        
        double cursorXOffset;

        if( cursorCentered ) {
            cursorXOffset = mWide / 2 - mBorderWide;
            }
        else if( leftAlign ) {
            cursorXOffset = mFont->measureString( textBeforeCursor );
            if( cursorXOffset == 0 ) {
                cursorXOffset -= pixWidth;
                }
            }
        else {
            double afterLength = mFont->measureString( textAfterCursor );
            cursorXOffset = ( mWide - 2 * mBorderWide ) - afterLength;

            if( afterLength > 0 ) {
                cursorXOffset -= pixWidth;
                }
            }
        

        
        delete [] beforeCursorText;
        
        setDrawColor( 0, 0, 0, 0.5 );
        
        drawRect( textPos.x + cursorXOffset, 
                  rectStartY - pixWidth,
                  textPos.x + cursorXOffset + pixWidth, 
                  rectEndY + pixWidth );
        }
    
    
    if( ! mActive ) {
        setDrawColor( 0, 0, 0, 0.5 );
        // dark overlay
        drawRect( - mWide / 2, - mHigh / 2, 
                  mWide / 2, mHigh / 2 );
        }
        

    delete [] textBeforeCursorBase;
    delete [] textAfterCursorBase;
    }


void TextField::pointerUp( float inX, float inY ) {
    if( mIgnoreMouse ) {
        return;
        }
    
    if( inX > - mWide / 2 &&
        inX < + mWide / 2 &&
        inY > - mHigh / 2 &&
        inY < + mHigh / 2 ) {

        char wasHidden = mContentsHidden;

        focus();

        if( wasHidden ) {
            // don't adjust cursor from where it was
            }
        else {
            
            int bestCursorDrawPosition = mCursorDrawPosition;
            double bestDistance = mWide * 2;
            
            int drawnTextLength = strlen( mDrawnText );
            
            // find gap between drawn letters that is closest to clicked x
            
            for( int i=0; i<=drawnTextLength; i+=mbclen(mDrawnText+i) ) {
                
                char *textCopy = stringDuplicate( mDrawnText );
                
                textCopy[i] = '\0';
                
                double thisGapX = 
                    mDrawnTextX + 
                    mFont->measureString( textCopy ) +
                    mFont->getCharSpacing() / 2;
                
                delete [] textCopy;
                
                double thisDistance = fabs( thisGapX - inX );
                
                if( thisDistance < bestDistance ) {
                    bestCursorDrawPosition = i;
                    bestDistance = thisDistance;
                    }
                if (mbclen(mDrawnText+i) == 0) {
                    break;
                    }
                }
            
            int cursorDelta = bestCursorDrawPosition - mCursorDrawPosition;
            
            mCursorPosition += cursorDelta;
            }
        }
    }


char furiganaSrc[][5] = {
    { "ltsu", },
    { "xtsu", },
    { "lwa", },
    { "kya", },
    { "kyu", },
    { "kyo", },
    { "sya", },
    { "syu", },
    { "syo", },
    { "sha", },
    { "shi", },
    { "shu", },
    { "sho", },
    { "tya", },
    { "tyu", },
    { "tyo", },
    { "tsu", },
    { "chi", },
    { "cha", },
    { "chu", },
    { "che", },
    { "cho", },
    { "nya", },
    { "nyu", },
    { "nyo", },
    { "hya", },
    { "hyu", },
    { "hyo", },
    { "mya", },
    { "myu", },
    { "myo", },
    { "rya", },
    { "ryu", },
    { "ryo", },
    { "lya", },
    { "lyi", },
    { "lyu", },
    { "lye", },
    { "lyo", },
    { "xya", },
    { "xyi", },
    { "xyu", },
    { "xye", },
    { "xyo", },
    { "xwa", },
    { "gya", },
    { "gyu", },
    { "gyo", },
    { "zya", },
    { "zyu", },
    { "zyo", },
    { "jya", },
    { "jyu", },
    { "jyo", },
    { "dya", },
    { "dyu", },
    { "dyo", },
    { "bya", },
    { "byu", },
    { "byo", },
    { "pya", },
    { "pyu", },
    { "pyo", },
    { "ltu", },
    { "xtu", },
    { "wha", },
    { "whi", },
    { "whu", },
    { "whe", },
    { "who", },
    { "qwa", },
    { "qwi", },
    { "qwu", },
    { "qwe", },
    { "qwo", },
    { "qya", },
    { "qyi", },
    { "qyu", },
    { "qye", },
    { "qyo", },
    { "kwa", },
    { "gwa", },
    { "gwi", },
    { "gwu", },
    { "gwe", },
    { "gwo", },
    { "swa", },
    { "swi", },
    { "swu", },
    { "swe", },
    { "swo", },
    { "twa", },
    { "twi", },
    { "twu", },
    { "twe", },
    { "two", },
    { "dwa", },
    { "dwi", },
    { "dwu", },
    { "dwe", },
    { "dwo", },
    { "fwa", },
    { "fwi", },
    { "fwu", },
    { "fwe", },
    { "fwo", },
    { "tha", },
    { "thi", },
    { "thu", },
    { "the", },
    { "tho", },
    { "dha", },
    { "dhi", },
    { "dhu", },
    { "dhe", },
    { "dho", },
    { "kye", },
    { "gyi", },
    { "gye", },
    { "syi", },
    { "sye", },
    { "she", },
    { "zyi", },
    { "zye", },
    { "tyi", },
    { "tye", },
    { "dyi", },
    { "dye", },
    { "nyi", },
    { "nye", },
    { "hyi", },
    { "hye", },
    { "byi", },
    { "bye", },
    { "pyi", },
    { "pye", },
    { "myi", },
    { "mye", },
    { "ryi", },
    { "rye", },
    { "xn", },
    { "ka", },
    { "ki", },
    { "ku", },
    { "ke", },
    { "ko", },
    { "sa", },
    { "si", },
    { "su", },
    { "se", },
    { "so", },
    { "ta", },
    { "ti", },
    { "tu", },
    { "te", },
    { "to", },
    { "na", },
    { "ni", },
    { "nu", },
    { "ne", },
    { "no", },
    { "ha", },
    { "hi", },
    { "hu", },
    { "he", },
    { "ho", },
    { "ma", },
    { "mi", },
    { "mu", },
    { "me", },
    { "mo", },
    { "ya", },
    { "yi", },
    { "yu", },
    { "ye", },
    { "yo", },
    { "ra", },
    { "ri", },
    { "ru", },
    { "re", },
    { "ro", },
    { "wa", },
    { "wi", },
    { "wu", },
    { "we", },
    { "wo", },
    { "va", },
    { "vi", },
    { "vu", },
    { "ve", },
    { "vo", },
    { "fa", },
    { "fi", },
    { "fu", },
    { "fe", },
    { "fo", },
    { "ga", },
    { "gi", },
    { "gu", },
    { "ge", },
    { "go", },
    { "za", },
    { "zi", },
    { "zu", },
    { "ze", },
    { "zo", },
    { "ja", },
    { "ji", },
    { "ju", },
    { "je", },
    { "jo", },
    { "da", },
    { "di", },
    { "du", },
    { "de", },
    { "do", },
    { "ba", },
    { "bi", },
    { "bu", },
    { "be", },
    { "bo", },
    { "pa", },
    { "pi", },
    { "pu", },
    { "pe", },
    { "po", },
    { "la", },
    { "li", },
    { "lu", },
    { "le", },
    { "lo", },
    { "xa", },
    { "xi", },
    { "xu", },
    { "xe", },
    { "xo", },
    { "nn", },
    { "ca", },
    { "ci", },
    { "cu", },
    { "ce", },
    { "co", },
    { "qa", },
    { "qi", },
    { "qu", },
    { "qe", },
    { "qo", },
    { "a", },
    { "i", },
    { "u", },
    { "e", },
    { "o", },
    { ".", },
    { ",", },
    { "!", },
    { "?", },
    { "-", },
    { "(", },
    { ")", },
    { "\"", },
    { "\'", },
    { "/", },
    { "^", },
    { "~", },
};
char furiganaDist[][7] = {
    { "っ", },
    { "っ", },
    { "ゎ", },
    { "きゃ", },
    { "きゅ", },
    { "きょ", },
    { "しゃ", },
    { "しゅ", },
    { "しょ", },
    { "しゃ", },
    { "し", },
    { "しゅ", },
    { "しょ", },
    { "ちゃ", },
    { "ちゅ", },
    { "ちょ", },
    { "つ", },
    { "ち", },
    { "ちゃ", },
    { "ちゅ", },
    { "ちぇ", },
    { "ちょ", },
    { "にゃ", },
    { "にゅ", },
    { "にょ", },
    { "ひゃ", },
    { "ひゅ", },
    { "ひょ", },
    { "みゃ", },
    { "みゅ", },
    { "みょ", },
    { "りゃ", },
    { "りゅ", },
    { "りょ", },
    { "ゃ", },
    { "ぃ", },
    { "ゅ", },
    { "ぇ", },
    { "ょ", },
    { "ゃ", },
    { "ぃ", },
    { "ゅ", },
    { "ぇ", },
    { "ょ", },
    { "ゎ", },
    { "ぎゃ", },
    { "ぎゅ", },
    { "ぎょ", },
    { "じゃ", },
    { "じゅ", },
    { "じょ", },
    { "じゃ", },
    { "じゅ", },
    { "じょ", },
    { "ぢゃ", },
    { "ぢゅ", },
    { "ぢょ", },
    { "びゃ", },
    { "びゅ", },
    { "びょ", },
    { "ぴゃ", },
    { "ぴゅ", },
    { "ぴょ", },
    { "っ", },
    { "っ", },
    { "うぁ", },
    { "うぃ", },
    { "う", },
    { "うぇ", },
    { "うぉ", },
    { "くぁ", },
    { "くぃ", },
    { "くぅ", },
    { "くぇ", },
    { "くぉ", },
    { "くゃ", },
    { "くぃ", },
    { "くゅ", },
    { "くぇ", },
    { "くょ", },
    { "くぁ", },
    { "ぐぁ", },
    { "ぐぃ", },
    { "ぐぅ", },
    { "ぐぇ", },
    { "ぐぉ", },
    { "すぁ", },
    { "すぃ", },
    { "すぅ", },
    { "すぇ", },
    { "すぉ", },
    { "とぁ", },
    { "とぃ", },
    { "とぅ", },
    { "とぇ", },
    { "とぉ", },
    { "どぁ", },
    { "どぃ", },
    { "どぅ", },
    { "どぇ", },
    { "どぉ", },
    { "ふぁ", },
    { "ふぃ", },
    { "ふぅ", },
    { "ふぇ", },
    { "ふぉ", },
    { "てゃ", },
    { "てぃ", },
    { "てゅ", },
    { "てぇ", },
    { "てょ", },
    { "でゃ", },
    { "でぃ", },
    { "でゅ", },
    { "でぇ", },
    { "でょ", },
    { "きぇ", },
    { "ぎぃ", },
    { "ぎぇ", },
    { "しぃ", },
    { "しぇ", },
    { "しぇ", },
    { "じぃ", },
    { "じぇ", },
    { "ちぃ", },
    { "ちぇ", },
    { "ぢぃ", },
    { "ぢぇ", },
    { "にぃ", },
    { "にぇ", },
    { "ひぃ", },
    { "ひぇ", },
    { "びぃ", },
    { "びぇ", },
    { "ぴぃ", },
    { "ぴぇ", },
    { "みぃ", },
    { "みぇ", },
    { "りぃ", },
    { "りぇ", },
    { "ん", },
    { "か", },
    { "き", },
    { "く", },
    { "け", },
    { "こ", },
    { "さ", },
    { "し", },
    { "す", },
    { "せ", },
    { "そ", },
    { "た", },
    { "ち", },
    { "つ", },
    { "て", },
    { "と", },
    { "な", },
    { "に", },
    { "ぬ", },
    { "ね", },
    { "の", },
    { "は", },
    { "ひ", },
    { "ふ", },
    { "へ", },
    { "ほ", },
    { "ま", },
    { "み", },
    { "む", },
    { "め", },
    { "も", },
    { "や", },
    { "い", },
    { "ゆ", },
    { "いぇ", },
    { "よ", },
    { "ら", },
    { "り", },
    { "る", },
    { "れ", },
    { "ろ", },
    { "わ", },
    { "うぃ", },
    { "う", },
    { "うぇ", },
    { "を", },
    { "ば", },
    { "び", },
    { "ぶ", },
    { "べ", },
    { "ぼ", },
    { "ふぁ", },
    { "ふぃ", },
    { "ふ", },
    { "ふぇ", },
    { "ふぉ", },
    { "が", },
    { "ぎ", },
    { "ぐ", },
    { "げ", },
    { "ご", },
    { "ざ", },
    { "じ", },
    { "ず", },
    { "ぜ", },
    { "ぞ", },
    { "じゃ", },
    { "じ", },
    { "じゅ", },
    { "じぇ", },
    { "じょ", },
    { "だ", },
    { "ぢ", },
    { "づ", },
    { "で", },
    { "ど", },
    { "ば", },
    { "び", },
    { "ぶ", },
    { "べ", },
    { "ぼ", },
    { "ぱ", },
    { "ぴ", },
    { "ぷ", },
    { "ぺ", },
    { "ぽ", },
    { "ぁ", },
    { "ぃ", },
    { "ぅ", },
    { "ぇ", },
    { "ぉ", },
    { "ぁ", },
    { "ぃ", },
    { "ぅ", },
    { "ぇ", },
    { "ぉ", },
    { "ん", },
    { "か", },
    { "し", },
    { "く", },
    { "せ", },
    { "こ", },
    { "くぁ", },
    { "くぃ", },
    { "く", },
    { "くぇ", },
    { "くぉ", },
    { "あ", },
    { "い", },
    { "う", },
    { "え", },
    { "お", },
    { "。", },
    { "、", },
    { "！", },
    { "？", },
    { "ー", },
    { "（", },
    { "）", },
    { "”", },
    { "’", },
    { "・", },
    { "＾", },
    { "～", },
};

unsigned char allowedFurigana[][4] = {
    { "’", },
    { "”", },
    { "、", },
    { "。", },
    { "ぁ", },
    { "あ", },
    { "ぃ", },
    { "い", },
    { "ぅ", },
    { "う", },
    { "ぇ", },
    { "え", },
    { "ぉ", },
    { "お", },
    { "か", },
    { "が", },
    { "き", },
    { "ぎ", },
    { "く", },
    { "ぐ", },
    { "け", },
    { "げ", },
    { "こ", },
    { "ご", },
    { "さ", },
    { "ざ", },
    { "し", },
    { "じ", },
    { "す", },
    { "ず", },
    { "せ", },
    { "ぜ", },
    { "そ", },
    { "ぞ", },
    { "た", },
    { "だ", },
    { "ち", },
    { "ぢ", },
    { "っ", },
    { "つ", },
    { "づ", },
    { "て", },
    { "で", },
    { "と", },
    { "ど", },
    { "な", },
    { "に", },
    { "ぬ", },
    { "ね", },
    { "の", },
    { "は", },
    { "ば", },
    { "ぱ", },
    { "ひ", },
    { "び", },
    { "ぴ", },
    { "ふ", },
    { "ぶ", },
    { "ぷ", },
    { "へ", },
    { "べ", },
    { "ぺ", },
    { "ほ", },
    { "ぼ", },
    { "ぽ", },
    { "ま", },
    { "み", },
    { "む", },
    { "め", },
    { "も", },
    { "ゃ", },
    { "や", },
    { "ゅ", },
    { "ゆ", },
    { "ょ", },
    { "よ", },
    { "ら", },
    { "り", },
    { "る", },
    { "れ", },
    { "ろ", },
    { "ゎ", },
    { "わ", },
    { "を", },
    { "ん", },
    { "・", },
    { "ー", },
    { "！", },
    { "（", },
    { "）", },
    { "？", },
    { "＾", },
    { "～", },
};

unsigned char* TextField::processCharacter( unsigned char* pInASCII ) {

    unsigned char processedChar = pInASCII[0];
        
    if( mForceCaps ) {
        processedChar = toupper( processedChar );
        }

    if( mForbiddenChars != NULL ) {
        int num = strlen( mForbiddenChars );
            
        for( int i=0; i<num; i++ ) {
            if( mForbiddenChars[i] == processedChar ) {
                return (unsigned char*)stringDuplicate( "" );
                }
            }
        }
        

    if( mAllowedChars != NULL ) {
        int num = strlen( mAllowedChars );
            
        char allowed = false;
            
        for( int i=0; i<num; i++ ) {
            if( mAllowedChars[i] == processedChar ) {
                allowed = true;
                break;
                }
            }

        if( mAllowHiragana ) {
            for ( u_int i=0; i<sizeof(allowedFurigana)/sizeof(allowedFurigana[0]); i++ ) {
                bool isAllowd = true;
                for ( int j=0; allowedFurigana[i][j] != '\0'; j++ ) {
                    if (pInASCII[j] != allowedFurigana[i][j]) {
                        isAllowd = false;
                        break;
                        }
                    }
                if( isAllowd ) {
                    allowed = true;
                    break;
                    }
                }
            }

        if( !allowed ) {
            return (unsigned char*)stringDuplicate( "" );
            }
        }
    else {
        // no allowed list specified 
        
        if( processedChar == '\r' ) {
            // \r only permitted if it is listed explicitly
            return (unsigned char*)stringDuplicate( "" );
            }
        }

    char returnStr[5] = { '\0', '\0', '\0', '\0', '\0', };
    int cLen = mbclen( (char*)pInASCII );
    for( int i=0; i<cLen; i++ ) {
        returnStr[i] = pInASCII[i];
        }
        
    return (unsigned char*)stringDuplicate( returnStr );
    }



void TextField::insertCharacter( unsigned char inASCII ) {
    
    if( isAnythingSelected() ) {
        // delete selected first
        deleteHit();
        }

    // add to it
    char *oldText = mText;
    
    if( mMaxLength != -1 &&
        mbslen( oldText ) >= (unsigned int) mMaxLength ) {
        // max length hit, don't add it
        return;
        }
    

    char *preCursor = stringDuplicate( mText );
    preCursor[ mCursorPosition ] = '\0';
    char *postCursor = &( mText[ mCursorPosition ] );
    
    mText = autoSprintf( "%s%c%s", 
                         preCursor, inASCII, postCursor );
    mTextLen = mbslen( mText );

    delete [] preCursor;
    
    delete [] oldText;
    
    mCursorPosition++;
    }



void TextField::insertString( char *inString ) {
    if( isAnythingSelected() ) {
        // delete selected first
        deleteHit();
        }
    
    // add to it
    char *oldText = mText;
    

    char *preCursor = stringDuplicate( mText );
    preCursor[ mCursorPosition ] = '\0';
    char *postCursor = &( mText[ mCursorPosition ] );
    
    mText = autoSprintf( "%s%s%s", 
                         preCursor, inString, postCursor );
    
    mTextLen = mbslen( mText );
    

    delete [] preCursor;
    
    delete [] oldText;
    
    mCursorPosition += strlen( inString );

    }



int TextField::getCursorPosition() {
    return mCursorPosition;
    }


void TextField::cursorReset() {
    mCursorPosition = 0;
    }



void TextField::setIgnoreArrowKeys( char inIgnore ) {
    mIgnoreArrowKeys = inIgnore;
    }



void TextField::setIgnoreMouse( char inIgnore ) {
    mIgnoreMouse = inIgnore;
    }



double TextField::getRightEdgeX() {
    
    return mX + mWide / 2;
    }



void TextField::setFireOnAnyTextChange( char inFireOnAny ) {
    mFireOnAnyChange = inFireOnAny;
    }


void TextField::setFireOnLoseFocus( char inFireOnLeave ) {
    mFireOnLeave = inFireOnLeave;
    }

int TextField::getPrevStringByteLen(int position) {
    int i = 0;
    int prevCharLen = 0;
    while (i < position) {
        prevCharLen = mbclen( mText + i );
        i+= prevCharLen;
        }
        /*
    int prevCharLen = 0;
    if( position < 2) {
        prevCharLen = 1;
        }
    else if( position == 2 ) {
        if( mbclen( (unsigned char*)mText + position - 1 ) != 1 ) {
            prevCharLen = mbclen( (unsigned char*)mText + position - 1 );
            }
        else{
            prevCharLen = 1;
            }
        }
    else if( mbclen( (unsigned char*)mText + position - 1 ) != 1 ) {
        if( mbclen( (unsigned char*)mText + position - 2 ) != 1 ) {
            prevCharLen = 1;
            }
        else{
            prevCharLen = mbclen( (unsigned char*)mText + position - 1 );
            }
        }
    else {
        prevCharLen = 1;
        }
        */

    /*
    AppLog::getLog()->logPrintf(Log::INFO_LEVEL, "mText %s, mCursorPosition %d, prevStringByteLen %d", mText, mCursorPosition, prevCharLen );
    for (int i=0; i<strlen(mText); i++) {
        AppLog::getLog()->logPrintf(Log::INFO_LEVEL, "mText(%d) mblen = %d", i, mblen(( char*)mText + i, MB_CUR_MAX) );
    }
    */
    return prevCharLen;
    }


void TextField::keyDown( unsigned char inASCII ) {
    if( !mFocused ) {
        return;
        }
    mCursorFlashSteps = 0;
    
    if( isCommandKeyDown() ) {
        // not a normal key stroke (command key)
        // ignore it as input

        // but ONLY if it's an alphabetical key (A-Z,a-z)
        // Some international keyboards use ALT to type certain symbols

        if( ( inASCII >= 'A' && inASCII <= 'Z' )
            ||
            ( inASCII >= 'a' && inASCII <= 'z' ) ) {
            
            return;
            }
        
        }
    

    if( inASCII == 127 || inASCII == 8 ) {
        // delete
        deleteHit();
        
        mHoldDeleteSteps = 0;

        clearArrowRepeat();
        }
    else if( inASCII == 13 ) {
        // enter hit in field
        unsigned char *pProcessedChar = processCharacter( &inASCII );    


        if( pProcessedChar[0] != 0 ) {
            // newline is allowed
            insertString( (char*)pProcessedChar );
            
            mHoldDeleteSteps = -1;
            mFirstDeleteRepeatDone = false;
            
            clearArrowRepeat();
            
            if( mFireOnAnyChange ) {
                fireActionPerformed( this );
                }
            }
        else {
            // newline not allowed in this field
            fireActionPerformed( this );
            }
        delete [] pProcessedChar;
        }
    else if( inASCII >= 32 ) {

        unsigned char *pProcessedChar = processCharacter( &inASCII );    

        bool isFirstSlash = ( strcmp( mText, "" ) == 0 ) && pProcessedChar[0] == '/';
        if( mAllowHiragana && !isFirstSlash ) {
            int pendingFuriganaLen = 0;
            for( int i = 0; i <= mCursorPosition; i++ ) {
                unsigned int cLen = mbclen( mText + i );
                if( cLen >= 2 ) {
                    i += cLen - 1;
                    pendingFuriganaLen = 0;
                    }
                else {
                    pendingFuriganaLen++;
                    }
                }

            for( size_t i = 0; i < sizeof(furiganaSrc)/sizeof(furiganaSrc[0]); i++) {
                int furiganaLen = strlen(furiganaSrc[i]);
                if( pendingFuriganaLen + 1 < furiganaLen ) {
                    continue;
                    }
                if (furiganaLen -1 <= mCursorPosition) {
                    for( int j = 0; j < furiganaLen; j++ ) {
                        char furigana = furiganaSrc[i][j];
                        if( j != furiganaLen - 1 ) {
                            if (mText[mCursorPosition - furiganaLen + j + 1] != furigana) {
                                break;
                                }
                            }
                        else{
                            if( furigana == pProcessedChar[0] ) {
                                for( int k = 0; k < furiganaLen - 1; k++ ) {
                                    deleteHit();
                                    }
                                insertString( furiganaDist[i] );
                                pProcessedChar[0] = '\0';
                                if( mMaxLength > 0 ) {
                                    while( mbslen( mText ) > mMaxLength ) {
                                        deleteHit();
                                        }
                                    }
                                }
                            }
                        }
                    }
                if( pProcessedChar[0] == 0 ) {
                    break;
                    }
                }
            }

        if( pProcessedChar[0] != 0 ) {
            
            insertString( (char*)pProcessedChar );
            if( mMaxLength > 0 ) {
                while( mbslen( mText ) > mMaxLength ) {
                    deleteHit();
                    }
                }
            }
        
        mHoldDeleteSteps = -1;
        mFirstDeleteRepeatDone = false;

        clearArrowRepeat();

        if( mFireOnAnyChange ) {
            fireActionPerformed( this );
            }

        delete [] pProcessedChar;
        }    
    }



void TextField::keyUp( unsigned char inASCII ) {
    if( inASCII == 127 || inASCII == 8 ) {
        // end delete hold down
        mHoldDeleteSteps = -1;
        mFirstDeleteRepeatDone = false;
        }
    }



void TextField::deleteHit() {
    if( mCursorPosition > 0 || isAnythingSelected() ) {
        mCursorFlashSteps = 0;
    
        int newCursorPos = mCursorPosition - getPrevStringByteLen(mCursorPosition);


        if( isAnythingSelected() ) {
            // selection delete
            
            mCursorPosition = mSelectionEnd;
            
            newCursorPos = mSelectionStart;

            mSelectionStart = -1;
            mSelectionEnd = -1;
            }
        else if( isCommandKeyDown() ) {
            // word delete 

            newCursorPos = mCursorPosition;

            // skip non-space, non-newline characters
            while( newCursorPos > 0 &&
                   mText[ newCursorPos - 1 ] != ' ' &&
                   mText[ newCursorPos - 1 ] != '\r' ) {
                newCursorPos --;
                }
        
            // skip space and newline characters
            while( newCursorPos > 0 &&
                   ( mText[ newCursorPos - 1 ] == ' ' ||
                     mText[ newCursorPos - 1 ] == '\r' ) ) {
                newCursorPos --;
                }
            }
        
        // section cleared no matter what when delete is hit
        mSelectionStart = -1;
        mSelectionEnd = -1;


        char *oldText = mText;
        
        char *preCursor = stringDuplicate( mText );
        preCursor[ newCursorPos ] = '\0';
        char *postCursor = &( mText[ mCursorPosition ] );

        mText = autoSprintf( "%s%s", preCursor, postCursor );
        mTextLen = mbslen( mText );
        
        delete [] preCursor;

        delete [] oldText;

        mCursorPosition = newCursorPos;

        if( mFireOnAnyChange ) {
            fireActionPerformed( this );
            }
        }
    }



void TextField::clearArrowRepeat() {
    for( int i=0; i<2; i++ ) {
        mHoldArrowSteps[i] = -1;
        mFirstArrowRepeatDone[i] = false;
        }
    }



void TextField::leftHit() {
    mCursorFlashSteps = 0;
    
    if( isShiftKeyDown() && mShiftPlusArrowsCanSelect ) {
        if( !isAnythingSelected() ) {
            mSelectionStart = mCursorPosition;
            mSelectionEnd = mCursorPosition;
            mSelectionAdjusting = &mSelectionStart;
            }
        else {
            mCursorPosition = *mSelectionAdjusting;
            }
        }

    if( ! isShiftKeyDown() ) {
        if( isAnythingSelected() ) {
            mCursorPosition = mSelectionStart + getPrevStringByteLen( mSelectionStart );
            }

        mSelectionStart = -1;
        mSelectionEnd = -1;
        }

    if( isCommandKeyDown() ) {
        // word jump 

        // skip non-space, non-newline characters
        while( mCursorPosition > 0 &&
               mText[ mCursorPosition - 1 ] != ' ' &&
               mText[ mCursorPosition - 1 ] != '\r' ) {
            mCursorPosition --;
            }
        
        // skip space and newline characters
        while( mCursorPosition > 0 &&
               ( mText[ mCursorPosition - 1 ] == ' ' ||
                 mText[ mCursorPosition - 1 ] == '\r' ) ) {
            mCursorPosition --;
            }
        
        }
    else {    
        mCursorPosition -= getPrevStringByteLen(mCursorPosition);
        if( mCursorPosition < 0 ) {
            mCursorPosition = 0;
            }
        }

    if( isShiftKeyDown() && mShiftPlusArrowsCanSelect ) {
        *mSelectionAdjusting = mCursorPosition;
        fixSelectionStartEnd();
        }

    }



void TextField::rightHit() {
    mCursorFlashSteps = 0;
    
    if( isShiftKeyDown() && mShiftPlusArrowsCanSelect ) {
        if( !isAnythingSelected() ) {
            mSelectionStart = mCursorPosition;
            mSelectionEnd = mCursorPosition;
            mSelectionAdjusting = &mSelectionEnd;
            }
        else {
            mCursorPosition = *mSelectionAdjusting;
            }
        }
    
    if( ! isShiftKeyDown() ) {
        if( isAnythingSelected() ) {
            mCursorPosition = mSelectionEnd - getPrevStringByteLen( mSelectionEnd );
            }
            
        mSelectionStart = -1;
        mSelectionEnd = -1;
        }

    if( isCommandKeyDown() ) {
        // word jump 
        int textLen = strlen( mText );
        
        // skip space and newline characters
        while( mCursorPosition < textLen &&
               ( mText[ mCursorPosition ] == ' ' ||
                 mText[ mCursorPosition ] == '\r'  ) ) {
            mCursorPosition ++;
            }

        // skip non-space and non-newline characters
        while( mCursorPosition < textLen &&
               mText[ mCursorPosition ] != ' ' &&
               mText[ mCursorPosition ] != '\r' ) {
            mCursorPosition ++;
            }
        
        
        }
    else {
        mCursorPosition += mbclen(mText + mCursorPosition);
        if( mCursorPosition > (int)strlen( mText ) ) {
            mCursorPosition = strlen( mText );
            }
        }

    if( isShiftKeyDown() && mShiftPlusArrowsCanSelect ) {
        *mSelectionAdjusting = mCursorPosition;
        fixSelectionStartEnd();
        }
    
    }




void TextField::specialKeyDown( int inKeyCode ) {
    if( !mFocused ) {
        return;
        }
    
    mCursorFlashSteps = 0;
    
    switch( inKeyCode ) {
        case MG_KEY_LEFT:
            if( ! mIgnoreArrowKeys ) {    
                leftHit();
                clearArrowRepeat();
                mHoldArrowSteps[0] = 0;
                }
            break;
        case MG_KEY_RIGHT:
            if( ! mIgnoreArrowKeys ) {
                rightHit(); 
                clearArrowRepeat();
                mHoldArrowSteps[1] = 0;
                }
            break;
        default:
            break;
        }
    
    }



void TextField::specialKeyUp( int inKeyCode ) {
    if( inKeyCode == MG_KEY_LEFT ) {
        mHoldArrowSteps[0] = -1;
        mFirstArrowRepeatDone[0] = false;
        }
    else if( inKeyCode == MG_KEY_RIGHT ) {
        mHoldArrowSteps[1] = -1;
        mFirstArrowRepeatDone[1] = false;
        }
    }



void TextField::focus() {
    
    if( sFocusedTextField != NULL && sFocusedTextField != this ) {
        // unfocus last focused
        sFocusedTextField->unfocus();
        }

    mFocused = true;
    sFocusedTextField = this;

    mContentsHidden = false;
    }



void TextField::unfocus() {
    mFocused = false;
 
    // hold-down broken if not focused
    mHoldDeleteSteps = -1;
    mFirstDeleteRepeatDone = false;

    clearArrowRepeat();

    if( sFocusedTextField == this ) {
        sFocusedTextField = NULL;
        if( mFireOnLeave ) {
            fireActionPerformed( this );
            }
        }    
    }



char TextField::isFocused() {
    return mFocused;
    }



void TextField::setDeleteRepeatDelays( int inFirstDelaySteps,
                                       int inNextDelaySteps ) {
    sDeleteFirstDelaySteps = inFirstDelaySteps;
    sDeleteNextDelaySteps = inNextDelaySteps;
    }



char TextField::isAnyFocused() {
    if( sFocusedTextField != NULL ) {
        return true;
        }
    return false;
    }


        
void TextField::unfocusAll() {
    
    if( sFocusedTextField != NULL ) {
        // unfocus last focused
        sFocusedTextField->unfocus();
        }

    sFocusedTextField = NULL;
    }




int TextField::getInt() {
    char *text = getText();
    
    int i = 0;
    
    sscanf( text, "%d", &i );
    
    delete [] text;
            
    return i;
    }

        
        
float TextField::getFloat() {
    char *text = getText();
            
    float f = 0;
    
    sscanf( text, "%f", &f );
    
    delete [] text;
    
    return f;
    }



void TextField::setInt( int inI ) {
    char *text = autoSprintf( "%d", inI );
    
    setText( text );
    delete [] text;
    }

        

void TextField::setFloat( float inF, int inDigitsAfterDecimal, 
                          char inTrimZeros ) {

    char *formatString;
    
    if( inDigitsAfterDecimal == -1 ) {
        formatString = stringDuplicate( "%f" );
        }
    else {
        formatString = autoSprintf( "%%.%df", inDigitsAfterDecimal );
        }

    char *text = autoSprintf( formatString, inF );
    
    if( inTrimZeros && strstr( text, "." ) != NULL ) {
        int index = strlen( text ) - 1;
        
        while( index > 1 && text[index] == '0' ) {
            if( text[index-1] == '.' ) {
                // leave one zero after .
                break;
                }
            text[index] = '\0';
            index --;
            }
        }

    delete [] formatString;

    setText( text );
    delete [] text;
    }




void TextField::setLabelSide( char inLabelOnRight ) {
    mLabelOnRight = inLabelOnRight;
    }



void TextField::setLabelTop( char inLabelOnTop ) {
    mLabelOnTop = inLabelOnTop;
    }


        
char TextField::isAnythingSelected() {
    return 
        ( mSelectionStart != -1 && 
          mSelectionEnd != -1 &&
          mSelectionStart != mSelectionEnd );
    }



char *TextField::getSelectedText() {

    if( ! isAnythingSelected() ) {
        return NULL;
        }
    
    char *textCopy = stringDuplicate( mText );

    textCopy[ mSelectionEnd ] = '\0';
    
    char *startPointer = &( textCopy[ mSelectionStart ] );
    
    char *returnVal = stringDuplicate( startPointer );
    
    delete [] textCopy;
    
    return returnVal;
    }



void TextField::fixSelectionStartEnd() {
    if( mSelectionEnd < mSelectionStart ) {
        int temp = mSelectionEnd;
        mSelectionEnd = mSelectionStart;
        mSelectionStart = temp;

        if( mSelectionAdjusting == &mSelectionStart ) {
            mSelectionAdjusting = &mSelectionEnd;
            }
        else if( mSelectionAdjusting == &mSelectionEnd ) {
            mSelectionAdjusting = &mSelectionStart;
            }
        }
    else if( mSelectionEnd == mSelectionStart ) {
        mSelectionAdjusting = &mSelectionEnd;
        }
    
    }



void TextField::setShiftArrowsCanSelect( char inCanSelect ) {
    mShiftPlusArrowsCanSelect = inCanSelect;
    }

