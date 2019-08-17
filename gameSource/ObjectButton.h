#ifndef OBJECT_BUTTON_INCLUDED
#define OBJECT_BUTTON_INCLUDED


#include "animationBank.h"

#include "Button.h"

#include "minorGems/game/gameGraphics.h"

class LiveObject;

class ObjectButton : public Button {
        
    public:
        
        ObjectButton( int objectId, double posX, double posY, double sizeX = 50, double sizeY = 50,
                      double offsetX = 0, double offsetY = 0, int animId = 0, bool animation = false,
                      const char *iBackgroundnTGAFileName = NULL, double inDrawScale = 1.0 );

        virtual ~ObjectButton();
        
        void setEmoteId( int emoteId );
        void setCaption( char* caption );
        void setLiveObject( LiveObject* liveObject );


    protected:
        int mObjectId;
        int mAnimId;
        int mEmoteId;
        bool mAnimation;
        double mSizeX;
        double mSizeY;
        double mOffsetX;
        double mOffsetY;
        SpriteHandle mBackgroundSprite;
        LiveObject* mLiveObject;

        double mDrawScale;
        double mAnimFrame;
        char* mCaption;
        
        // override
        virtual void draw();

        virtual void step();
        
    };



#endif
        
