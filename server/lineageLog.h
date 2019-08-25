

void initLineageLog();


void freeLineageLog();

void stepLineageLog();
void stepBirthLog();
void stepNamedLog();


void recordPlayerLineage( char *inEmail, double inAge,
                          int inPlayerID, int inParentID,
                          int inDisplayID, int inKillerID,
                          const char *inName,
                          const char *inLastSay,
                          char inMale,
                          int inYumChain );

void recordPlayerNamed( char *inEmail,
                          int inPlayerID,
                          const char *inName);

void recordPlayerBirth( char *inEmail,
                          int inPlayerID,
                          int inParentID,
                          int inDisplayID,
                          char inMale );


