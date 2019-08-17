#include "ObjectButton.h"

#include "minorGems/game/drawUtils.h"
#include "minorGems/game/font.h"
#include "minorGems/util/log/AppLog.h"
#include "minorGems/util/stringUtils.h"

#include "ageControl.h"
#include "emotion.h"
#include "objectBank.h"
#include "LivingLifePage.h"

extern double frameRateFactor;
extern float gui_fov_scale_hud;
extern Font* littleFont;

static SpriteHandle defaultObjectButtonBackgroundSprite = NULL;

ObjectButton::ObjectButton( int objectId, double inX, double inY, double sizeX, double sizeY, double offsetX, double offsetY, int animId, bool animation,
              const char *iBackgroundnTGAFileName, double inDrawScale )
        : Button( inX, inY, 
                  sizeX,
                  sizeY, 
                  inDrawScale ), 
          mObjectId( objectId ),
          mSizeX( sizeX ),
          mSizeY( sizeY ),
          mOffsetX( offsetX ),
          mOffsetY( offsetY ),
          mAnimId( animId ),
          mAnimation( animation ),
          mAnimFrame( 0 ),
          mDrawScale( inDrawScale ),
          mEmoteId( -1 ),
          mCaption( NULL ),
          mLiveObject( NULL ) {

    if( iBackgroundnTGAFileName ){
        Image *image = readTGAFile( iBackgroundnTGAFileName );

        if( image != NULL ) {
            // fill Button's values here
            mWide = image->getWidth() * 2 * inDrawScale;
            mHigh = image->getHeight() * 2 * inDrawScale;
            
            mBackgroundSprite = fillSprite( image );
            
            delete image;
            }
        else {
            AppLog::errorF( "Failed to read file for ObjectButton: %s",
                            iBackgroundnTGAFileName );
            }
        }
    else {
        if( defaultObjectButtonBackgroundSprite == NULL) {
            Image *image = readTGAFile( "objectButtonBackground.tga" );

            if( image != NULL ) {
                defaultObjectButtonBackgroundSprite = fillSprite( image );
                delete image;
                }
            }
        mBackgroundSprite = defaultObjectButtonBackgroundSprite;
        }
    }



ObjectButton::~ObjectButton() {
    if( mBackgroundSprite && mBackgroundSprite != defaultObjectButtonBackgroundSprite ) {
        freeSprite( mBackgroundSprite );
        }
    if( mCaption != NULL ) {
        delete [] mCaption;
        }
    }

void ObjectButton::setEmoteId( int emoteId ) {
    mEmoteId = emoteId;
    }

void ObjectButton::setLiveObject( LiveObject* liveObject ) {
    mLiveObject = liveObject;
    }

void ObjectButton::setCaption( char* caption ) {
    if( mCaption != NULL ) {
        delete [] mCaption;
        }
    mCaption = stringDuplicate( caption );    
    }

void ObjectButton::step() {
    mAnimFrame += 1 * frameRateFactor / 60.0;
    }


void ObjectButton::draw() {
    if( mBackgroundSprite != NULL ) {
        
//        setDrawColor( 1, 1, 1, 1 );
        doublePair center = { 0, 0 };
        drawSprite( mBackgroundSprite, center, mSizeX / getSpriteWidth(mBackgroundSprite) );
        }

    startAddingToStencil( true, true, 0.1f );
    
    if( mHover && ! mDragOver ) {
        setDrawColor( 0.75, 0.75, 0.75, 1 );
        }
    else if( mDragOver ) {
        setDrawColor( 0.5, 0.5, 0.5, 1 );
        }
    else {
        setDrawColor( 1, 1, 1, 1 );
        }
    doublePair center = { 0, 0 };
    if( mBackgroundSprite != NULL ) {
        drawSprite( mBackgroundSprite, center, mSizeX / getSpriteWidth(mBackgroundSprite) );
        }
    else {
        drawRect( center, mSizeX / 2, mSizeY / 2 );
        }
    
    startDrawingThroughStencil( false );

    doublePair pos = { mOffsetX, mOffsetY, };

    double age = -1;
    int objectId = mObjectId;

    if( mEmoteId >= 0 && mLiveObject != NULL ) {
        age = mLiveObject->age;
        objectId = mLiveObject->displayID;

        ObjectRecord* faceO = getObject( objectId );
        int headIndex = getHeadIndex( faceO, age );
        doublePair headPos = faceO->spritePos[ headIndex ];
        
        int frontFootIndex = getFrontFootIndex( faceO, age );
        doublePair frontFootPos = 
            faceO->spritePos[ frontFootIndex ];
        
        int bodyIndex = getBodyIndex( faceO, age );
        doublePair bodyPos = faceO->spritePos[ bodyIndex ];

        doublePair framePos = 
            add( add( faceO->spritePos[ headIndex ],
                      getAgeHeadOffset( age, headPos,
                                        bodyPos,
                                        frontFootPos ) ),
                 getAgeBodyOffset( age, bodyPos ) );
        
        // move face down
        pos.x -= 0;
        pos.y -= 16;
        pos = sub( pos, mult( framePos, mDrawScale ) );

        setAnimationEmotion( getEmotion( mEmoteId ) );
        }

    char used;
    drawObjectAnim( objectId, 2, 
                    ground, mAnimFrame,
                    0,
                    ground, 
                    mAnimFrame,
                    mAnimFrame,
                    &used,
                    ground,
                    ground,
                    pos, 0,
                    false,
                    false, age,
                    false, false, false,
                    mLiveObject != NULL ? mLiveObject->clothing : getEmptyClothingSet(),
                    mLiveObject != NULL ? mLiveObject->clothingContained : NULL,
                    NULL, NULL,
                    mDrawScale );
    if( mEmoteId >= 0) {
        setAnimationEmotion( NULL );
        }

    if( mCaption != NULL ) {
        doublePair textPos = { mSizeX / -2 + 3, mSizeY / 2 - 3 -8, };

        float prevScale = gui_fov_scale_hud;
        gui_fov_scale_hud = 1.0f;
        
        setDrawColor( 0, 0, 0, 1 );
        littleFont->drawString( mCaption, textPos, alignLeft );

        gui_fov_scale_hud = prevScale;
        }
    stopStencil();
    }
