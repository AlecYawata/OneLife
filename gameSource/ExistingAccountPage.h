#include "GamePage.h"

#include "TextField.h"
#include "TextButton.h"
#include "KeyEquivalentTextButton.h"


#include "minorGems/ui/event/ActionListener.h"


class ExistingAccountPage : public GamePage, public ActionListener {
        
    public:
        
        ExistingAccountPage();
        
        virtual ~ExistingAccountPage();
        
        void clearFields();


        // defaults to true
        void showReviewButton( char inShow );
        
        // defaults to false
        void showDisableCustomServerButton( char inShow );
        

        
        virtual void actionPerformed( GUIComponent *inTarget );

        
        virtual void makeActive( char inFresh );
        virtual void makeNotActive();

        virtual void step();
        

        // for TAB and ENTER (switch fields and start login)
        virtual void keyDown( unsigned char inASCII );
        
        // for arrow keys (switch fields)
        virtual void specialKeyDown( int inKeyCode );
        
        virtual void draw( doublePair inViewCenter, 
                           double inViewSize );


    protected:
        
        TextField mEmailField;
        TextField mKeyField;

        TextField *mFields[2];

        TextButton mAtSignButton;

        KeyEquivalentTextButton mPasteButton;

        TextButton mDisableCustomServerButton;
        
        TextButton mLoginButton;
        TextButton mFriendsButton;
        TextButton mFamilyTreesButton;
        TextButton mClearAccountButton;
        TextButton mCancelButton;

        TextButton mTutorial1Button;
        TextButton mTutorial2Button;
        TextButton mTutorial3Button;
        TextButton mTutorial4Button;
        TextButton mTutorial5Button;
        TextButton mTutorial6Button;
        TextButton mTutorial7Button;

#define TUTORIAL_MAX 7
        TextButton* mTutorialButtons[TUTORIAL_MAX];

        TextButton mSettingsButton;
        TextButton mReviewButton;
        
        TextButton mRedetectButton;

        TextButton mViewAccountButton;
        

        double mPageActiveStartTime;
        int mFramesCounted;
        char mFPSMeasureDone;

        char mHideAccount;

        void switchFields();
        
        void processLogin( char inStore, const char *inSignal );

    };

