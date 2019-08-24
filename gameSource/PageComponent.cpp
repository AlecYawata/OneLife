#include "PageComponent.h"

#include "minorGems/game/game.h"


extern float gui_fov_scale;

PageComponent::PageComponent( double inX, double inY )
        : mX( inX ), mY( inY ), mParent( NULL ), mVisible( true ),
          mIgnoreEvents( false ),
          mIgnoreViewCenter( true ),
          mMouseEventHog( NULL ) {
    
    }
        


doublePair PageComponent::getCenter() {
    doublePair c = { mX, mY };

    return c;
    }



void PageComponent::setParent( PageComponent *inParent ) {
    mParent = inParent;
    }



void PageComponent::setToolTip( const char *inTip ) {
    if( mParent != NULL ) {
        mParent->setToolTip( inTip );
        }
    }



void PageComponent::clearToolTip( const char *inTipToClear ) {
    if( mParent != NULL ) {
        mParent->clearToolTip( inTipToClear );
        }
    }



void PageComponent::base_step(){
    for( int i=0; i<mComponents.size(); i++ ) {
        PageComponent *c = *( mComponents.getElement( i ) );
        
        if( c->isVisible() && c->isActive() ) {
            c->base_step();
            }
        }
    
    step();
    }



void PageComponent::base_draw( doublePair inViewCenter, 
                               double inViewSize ){

    doublePair oldViewCenter = getViewCenterPosition();
    setViewCenterPosition( inViewCenter.x - mX, inViewCenter.y - mY );

    if( mIgnoreViewCenter ) {
        setLetterbox( 1280, 720 );
        setViewSize( 1280 );
//        setViewCenterPosition( -mX, -mY );
        }
    for( int i=0; i<mComponents.size(); i++ ) {
        PageComponent *c = *( mComponents.getElement( i ) );
    
        if( c->isVisible() ) {
            doublePair childViewCenter = { inViewCenter.x - mX, inViewCenter.y - mY, };
            c->base_draw( childViewCenter, inViewSize );
            }
        }

    draw();

    if( mIgnoreViewCenter ) {
        setLetterbox( 1280 * gui_fov_scale, 720 * gui_fov_scale );
        setViewSize( 1280 * gui_fov_scale );
        }
    setViewCenterPosition( oldViewCenter.x, oldViewCenter.y );
    }



void PageComponent::setPosition( double inX, double inY ) {
    mX = inX;
    mY = inY;
    }



doublePair PageComponent::getPosition() {
    doublePair p;
    p.x = mX;
    p.y = mY;
    return p;
    }



void PageComponent::base_clearState(){
    
    mMouseEventHog = NULL;

    for( int i=0; i<mComponents.size(); i++ ) {
        PageComponent *c = *( mComponents.getElement( i ) );

        c->base_clearState();
        }


    clearState();
    }



void PageComponent::setIgnoreEvents( char inIgnoreEvents ) {
    mIgnoreEvents = inIgnoreEvents;
    }



bool PageComponent::base_pointerMove( float inX, float inY ){
    bool focused = false;
    if( mIgnoreEvents ) {
        return false;
        }
    
    doublePair oldViewCenter = getViewCenterPosition();
    if( mIgnoreViewCenter ) {
        setViewCenterPosition( 0, 0 );
        inX -= oldViewCenter.x;
        inY -= oldViewCenter.y;
        inX /= gui_fov_scale;
        inY /= gui_fov_scale;
        }    
    inX -= mX;
    inY -= mY;

    if( mMouseEventHog != NULL ) {
        if( mMouseEventHog->isVisible() && mMouseEventHog->isActive() ) {
            mMouseEventHog->base_pointerMove( inX, inY );
            }
        }
    else {    
        for( int i=0; i<mComponents.size(); i++ ) {
            PageComponent *c = *( mComponents.getElement( i ) );
            
            if( c->isVisible() && c->isActive() ) {
                focused = c->base_pointerMove( inX, inY );
                if( focused ) {
                    break;
                    }
                }
            }
        }

    if( !focused ) {
        mFocused = false;
        pointerMove( inX, inY );
        focused = mFocused;
        }    
    setViewCenterPosition( oldViewCenter.x, oldViewCenter.y );
    return focused;
    }



bool PageComponent::base_pointerDown( float inX, float inY ){
    bool focused = false;
    if( mIgnoreEvents ) {
        return false;
        }

    doublePair oldViewCenter = getViewCenterPosition();
    if( mIgnoreViewCenter ) {
        setViewCenterPosition( 0, 0 );
        inX -= oldViewCenter.x;
        inY -= oldViewCenter.y;
        inX /= gui_fov_scale;
        inY /= gui_fov_scale;
        }
    inX -= mX;
    inY -= mY;

    if( mMouseEventHog != NULL ) {
        if( mMouseEventHog->isVisible() && mMouseEventHog->isActive() ) {
            mMouseEventHog->base_pointerDown( inX, inY );
            }
        }
    else { 
        for( int i=0; i<mComponents.size(); i++ ) {
            PageComponent *c = *( mComponents.getElement( i ) );
            
            if( c->isVisible() && c->isActive() ) {
                focused = c->base_pointerDown( inX, inY );
                if( focused ) {
                    break;
                    }
                }
            }
        }
    
    if( !focused ) {
        mFocused = false;
        pointerDown( inX, inY );
        focused = mFocused;
        }    
    setViewCenterPosition( oldViewCenter.x, oldViewCenter.y );
    return focused;
    }



bool PageComponent::base_pointerDrag( float inX, float inY ){
    bool focused = false;
    if( mIgnoreEvents ) {
        return false;
        }
    
    doublePair oldViewCenter = getViewCenterPosition();
    if( mIgnoreViewCenter ) {
        setViewCenterPosition( 0, 0 );
        inX -= oldViewCenter.x;
        inY -= oldViewCenter.y;
        inX /= gui_fov_scale;
        inY /= gui_fov_scale;
        }
    inX -= mX;
    inY -= mY;

    if( mMouseEventHog != NULL ) {
        if( mMouseEventHog->isVisible() && mMouseEventHog->isActive() ) {
            mMouseEventHog->base_pointerDrag( inX, inY );
            }
        }
    else {
        for( int i=0; i<mComponents.size(); i++ ) {
            PageComponent *c = *( mComponents.getElement( i ) );
            
            if( c->isVisible() && c->isActive() ) {
                focused = c->base_pointerDrag( inX, inY );
                if( focused ) {
                    break;
                    }
                }
            }
        }

    if( !focused ) {
        mFocused = false;
        pointerDrag( inX, inY );
        focused = mFocused;
        }    
    setViewCenterPosition( oldViewCenter.x, oldViewCenter.y );
    return focused;
    }



bool PageComponent::base_pointerUp( float inX, float inY ){
    bool focused = false;

    doublePair oldViewCenter = getViewCenterPosition();
    if( mIgnoreViewCenter ) {
        setViewCenterPosition( 0, 0 );
        inX -= oldViewCenter.x;
        inY -= oldViewCenter.y;
        inX /= gui_fov_scale;
        inY /= gui_fov_scale;
        }
    inX -= mX;
    inY -= mY;

    if( mMouseEventHog != NULL ) {
        if( mMouseEventHog->isVisible() && mMouseEventHog->isActive() ) {
            mMouseEventHog->base_pointerUp( inX, inY );
            }
        }
    else {
        for( int i=0; i<mComponents.size(); i++ ) {
            PageComponent *c = *( mComponents.getElement( i ) );
            
            if( c->isVisible() && c->isActive() ) {
                focused = c->base_pointerUp( inX, inY );
                if( focused ) {
                    break;
                    }
                }
            }
        }

    if( !focused ) {
        mFocused = false;
        pointerUp( inX, inY );
        focused = mFocused;
        }    
    setViewCenterPosition( oldViewCenter.x, oldViewCenter.y );
    return focused;
    }



void PageComponent::base_keyDown( unsigned char inASCII ){
    if( mIgnoreEvents ) {
        return;
        }
    
    for( int i=0; i<mComponents.size(); i++ ) {
        PageComponent *c = *( mComponents.getElement( i ) );
    
        if( c->isVisible() && c->isActive() ) {
            c->base_keyDown( inASCII );
            }
        }

    keyDown( inASCII );
    }


        
void PageComponent::base_keyUp( unsigned char inASCII ){
    for( int i=0; i<mComponents.size(); i++ ) {
        PageComponent *c = *( mComponents.getElement( i ) );
    
        if( c->isVisible() && c->isActive() ) {
            c->base_keyUp( inASCII );
            }
        }

    keyUp( inASCII );
    }



void PageComponent::base_specialKeyDown( int inKeyCode ){
    if( mIgnoreEvents ) {
        return;
        }
    
    for( int i=0; i<mComponents.size(); i++ ) {
        PageComponent *c = *( mComponents.getElement( i ) );
    
        if( c->isVisible() && c->isActive() ) {
            c->base_specialKeyDown( inKeyCode );
            }
        }

    specialKeyDown( inKeyCode );
    }



void PageComponent::base_specialKeyUp( int inKeyCode ){
    for( int i=0; i<mComponents.size(); i++ ) {
        PageComponent *c = *( mComponents.getElement( i ) );
    
        if( c->isVisible() && c->isActive() ) {
            c->base_specialKeyUp( inKeyCode );
            }
        }

    specialKeyUp( inKeyCode );
    }



void PageComponent::addComponent( PageComponent *inComponent ){

    mComponents.push_back( inComponent );

    inComponent->setParent( this );
    }


void PageComponent::removeComponent( PageComponent *inComponent ){

    mComponents.deleteElementEqualTo( inComponent );

    inComponent->setParent( NULL );
    }



void PageComponent::setWaiting( char inWaiting,
                                char inWarningOnly ) {
    // pass up chain (stops at GamePage)
    if( mParent != NULL ) {
        mParent->setWaiting( inWaiting, inWarningOnly );
        }
    }



void PageComponent::setHogMouseEvents( char inHogMouseEvents ) {
    PageComponent *newHog = this;
    
    
    if( ! inHogMouseEvents ) {
        newHog = NULL;
        }
    
    if( mParent != NULL ) {
        mParent->setMouseEventHog( newHog );
        }
    }



void PageComponent::setMouseEventHog( PageComponent *inHog ) {
    mMouseEventHog = inHog;
    }
