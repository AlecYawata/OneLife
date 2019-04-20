#include "GamePage.h"

#include "TextButton.h"

#include "minorGems/ui/event/ActionListener.h"




class TutorialPage : public GamePage, public ActionListener {
        
    public:
        
        TutorialPage();
        ~TutorialPage();
        

        virtual void draw( doublePair inViewCenter, 
                           double inViewSize );

        virtual void actionPerformed( GUIComponent *inTarget );
        
        virtual void makeActive( char inFresh );
        virtual void makeNotActive();

    protected:

        TextButton mBackButton;
        TextButton mTutorial1Button;
        TextButton mTutorial2Button;
        TextButton mTutorial3Button;
        TextButton mTutorial4Button;
        TextButton mTutorial5Button;
        TextButton mTutorial6Button;
        TextButton mTutorial7Button;
        TextButton mTutorial8Button;
        TextButton mTutorial9Button;
        TextButton mTutorial10Button;

#define TUTORIAL_MAX 10
        TextButton* mTutorialButtons[TUTORIAL_MAX];

    };
