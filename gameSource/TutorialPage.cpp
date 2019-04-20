#include "TutorialPage.h"


#include "minorGems/game/Font.h"

#include "minorGems/util/SettingsManager.h"
#include "minorGems/util/stringUtils.h"

#include "minorGems/game/game.h"

#include "buttonStyle.h"

extern Font *mainFont;
extern Font *mainFontTutorial;



TutorialPage::TutorialPage()
        : mBackButton( mainFont, 0, -400, translate( "backButton" ) ),
          mTutorial2Button( mainFont, -200, 160, translate( "tutorial2" ) ),
          mTutorial1Button( mainFont, -200, 80, translate( "tutorial1" ) ),
          mTutorial4Button( mainFont, -200, 0, translate( "tutorial4" ) ),
          mTutorial5Button( mainFont, -200, -80, translate( "tutorial5" ) ),
          mTutorial3Button( mainFont, -200, -160, translate( "tutorial3" ) ),
          mTutorial6Button( mainFont, 200, 160, translate( "tutorial6" ) ),
          mTutorial7Button( mainFont, 200, 80, translate( "tutorial7" ) ),
          mTutorial8Button( mainFont, 200, 0, translate( "tutorial8" ) ),
          mTutorial9Button( mainFont, 200, -80, translate( "tutorial9" ) ),
          mTutorial10Button( mainFont, 200, -160, translate( "tutorial10" ) ) {

    mTutorialButtons[0] = &mTutorial1Button;
    mTutorialButtons[1] = &mTutorial2Button;
    mTutorialButtons[2] = &mTutorial3Button;
    mTutorialButtons[3] = &mTutorial4Button;
    mTutorialButtons[4] = &mTutorial5Button;
    mTutorialButtons[5] = &mTutorial6Button;
    mTutorialButtons[6] = &mTutorial7Button;
    mTutorialButtons[7] = &mTutorial8Button;
    mTutorialButtons[8] = &mTutorial9Button;
    mTutorialButtons[9] = &mTutorial10Button;
    for (int i=0; i<TUTORIAL_MAX; i++) {
        setButtonStyle( mTutorialButtons[i] );
        addComponent( mTutorialButtons[i] );
        (*mTutorialButtons[i]).addActionListener( this );
        }

    setButtonStyle( &mBackButton );
    addComponent( &mBackButton );
    mBackButton.addActionListener( this );

    }



TutorialPage::~TutorialPage() {

    }


void TutorialPage::actionPerformed( GUIComponent *inTarget ) {
    if( inTarget == &mBackButton ) {
        setSignal( "back" );
        }
    for (int i=0; i<TUTORIAL_MAX; i++) {
        if( inTarget == mTutorialButtons[i] ) {
            SettingsManager::setSetting( "tutorialNumber", i+1 );
            setSignal( "tutorial" );
            }
        }    
    }



void TutorialPage::draw( doublePair inViewCenter, 
                         double inViewSize ) {

    }


void TutorialPage::makeActive( char inFresh ) {
    if( inFresh ) {        
        }
    }



void TutorialPage::makeNotActive() {
    }

