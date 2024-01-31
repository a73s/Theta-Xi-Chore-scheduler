/*
Author: Adam Seals
Date: 1/12/24
*/

#include <cstdlib>
#include <vector>
#include <string>
#include <ctime>
//#include <cstdio>

#include "func.h"
#include "person.h"
#include "file.h"

int genRand(const int min, const int max){

    return (rand()%(max-min+1)+min);
}

void readInData(std::vector <Detail> & communityDetails, std::vector <Detail> & newHouseDetails, std::vector <Detail> & oldHouseDetails, std::vector <Person> & newHouse, std::vector <Person> & oldHouse){

    //these are for details
    ReadFile communityDetailFile;
    ReadFile newHouseDetailFile;
    ReadFile oldHouseDetailFile;

    //these files are for reading in names
    ReadFile eboardsFile;
    ReadFile newHouseFile;
    ReadFile oldHouseFile;
    ReadFile pledgesFile;
    ReadFile graduatingFile;

    communityDetailFile.Open(COMMUNITY_DETAIL_FILE);
    newHouseDetailFile.Open(NEW_HOUSE_DETAIL_FILE);
    oldHouseDetailFile.Open(OLD_HOUSE_DETAIL_FILE);

    eboardsFile.Open(EBOARDS_FILE);
    newHouseFile.Open(NEW_HOUSE_FILE);
    oldHouseFile.Open(OLD_HOUSE_FILE);
    pledgesFile.Open(PLEDGES_FILE);
    graduatingFile.Open(GRADUATING_FILE);

    //these are for the purpose of determining if people are pledges/eboards/graduating when putting them into the new/old house lists of people
    //it may be costly to loop through each of these vectors every time we need to create a person but its a cost im willing to accept because
    //this paradigm allows the user to put a name in every file which is applicable to that person, vastly simplifying the process of configuration
    std::vector <Person> eboards;
    std::vector <Person> pledges;
    std::vector <Person> graduating;

    std::string fileLine;

    //skip the first line of each file because there is a descriptor line there
    fileLine = communityDetailFile.readLine();
    fileLine = newHouseDetailFile.readLine();
    fileLine = oldHouseDetailFile.readLine();

    //input the details into the lists
    while(!communityDetailFile.eof()){

        fileLine = communityDetailFile.readLine();
        
        if(fileLine.empty()){
            
            continue;
        }

        communityDetails.push_back(Detail(fileLine));
    }

    while(!newHouseDetailFile.eof()){

        fileLine = newHouseDetailFile.readLine();
        
        if(fileLine.empty()){
            
            continue;
        }
        
        newHouseDetails.push_back(Detail(fileLine));
    }

    while(!oldHouseDetailFile.eof()){

        fileLine = oldHouseDetailFile.readLine();
        
        if(fileLine.empty()){
            
            continue;
        }
        
        oldHouseDetails.push_back(Detail(fileLine));
    }

    //skip first line, it is a title
    fileLine = eboardsFile.readLine();
    fileLine = newHouseFile.readLine();
    fileLine = oldHouseFile.readLine();
    fileLine = pledgesFile.readLine();
    fileLine = graduatingFile.readLine();

    //read in the names into eboard/pledge/grad. This allows us to easily check if they are eboard or pledge or grad when it comes to new/old house lists
    while(!graduatingFile.eof()){

        fileLine = graduatingFile.readLine();

        if(!fileLine.empty()){

            graduating.push_back(Person(fileLine));//since the equality operator for person only checks the name, we can skip setting the other variables
        }
    }

    while(!pledgesFile.eof()){

        fileLine = pledgesFile.readLine();

        std::string inFile = "";

        if(!fileLine.empty()){

            int section = 0;

            //we must exclude anything after the ':' because the pledge timer is there
            for(int i = 0; i < static_cast<int>(fileLine.size()); i++){

                if(fileLine[i] == ':'){

                    section++;
                    continue;
                }
                
                if(section == 0){

                    inFile += fileLine[i];
                }
            }

            pledges.push_back(Person(inFile));
        }
    }

    while(!eboardsFile.eof()){

        fileLine = eboardsFile.readLine();

        if(!fileLine.empty()){

            eboards.push_back(Person(fileLine));
        }
    }

    while(!newHouseFile.eof()){

        fileLine = newHouseFile.readLine();

        if(!fileLine.empty()){

            bool isPledge = isInVector<Person>(pledges, Person(fileLine, false, true, false, ""));
            bool isEboard = isInVector<Person>(eboards, Person(fileLine, true, false, false, ""));
            bool isGraduating = isInVector<Person>(graduating, Person(fileLine, false, false, true, ""));

            newHouse.push_back(Person(fileLine, isEboard, isPledge, isGraduating, NEW_HOUSE_TITLE));
        }
    }

    while(!oldHouseFile.eof()){

        fileLine = oldHouseFile.readLine();

        if(!fileLine.empty()){

            bool isPledge = isInVector<Person>(pledges, Person(fileLine, false, true, false, ""));
            bool isEboard = isInVector<Person>(eboards, Person(fileLine, true, false, false, ""));
            bool isGraduating = isInVector<Person>(graduating, Person(fileLine, false, false, true, ""));

            oldHouse.push_back(Person(fileLine, isEboard, isPledge, isGraduating, OLD_HOUSE_TITLE));
        }
    }

    //close the files
    communityDetailFile.Close();
    newHouseDetailFile.Close();
    oldHouseDetailFile.Close();
    eboardsFile.Close();
    newHouseFile.Close();
    oldHouseFile.Close(); 

    return;
}

void managePledgeTimer(const int timeDays){

    ReadFile pledgesFile;
    pledgesFile.Open(PLEDGES_FILE);
    std::string fileLine;
    std::vector<std::string> fileLinesBuffer;//vector from which we will recreate the file

    fileLine = pledgesFile.readLine();//skip first line, its not a name
    fileLinesBuffer.push_back(fileLine);

    //parse each line then check if they need a date added or if they need to be removed
    while(!pledgesFile.eof()){

        std::string name = "";
        std::string startTime = "";

        fileLine = pledgesFile.readLine();

        if(fileLine.empty()){
        
            continue;
        }

        fileLinesBuffer.push_back(fileLine);
        int section = 0;

        for(int i = 0; i < static_cast<int>(fileLine.size()); i++){

            if(fileLine[i] == ':'){

                section++;
                continue;
            }
                
            switch(section){

                case 0:{

                    name += fileLine[i];
                    break;
                }
                case 1:{

                    if(fileLine[i] != ' '){
                        startTime += fileLine[i];
                    }
                    break;
                }
            }
        }

        //adding pledge timer, or removing him from file if 
        if(startTime.empty()){

            std::string tempstr = fileLinesBuffer[pledgesFile.LineNum() - 1];
            fileLinesBuffer.pop_back();
            fileLinesBuffer.push_back(tempstr + ':' + std::to_string(time(NULL)));

        }else{

            int pledgeEndTime = stoi(startTime) + (timeDays * SECONDS_IN_DAY);

            if(time(NULL) >= pledgeEndTime){
            
                fileLinesBuffer.pop_back();
            }
        }
    }

    pledgesFile.Close();

    WriteFile pledgesWrite;
    pledgesWrite.Open(PLEDGES_FILE);

    for(int i = 0; i < static_cast<int>(fileLinesBuffer.size()); i++){

        pledgesWrite.WriteLine(fileLinesBuffer[i], '\n');
    }

    pledgesWrite.Close();

    return;
}

Person randomPerson(std::vector<Person> & persons, const bool canBeEboard, const bool canBePledge, const std::string targetHouse){

    std::vector<Person *> qualifiedPersons;//vector of pointers so we're not copying so much data

    bool isQualified = false;

    // we want to create a list of people who are elegable for the given detail spot.
    // this allows us to pick people from the list and be certain that they are elegable.
    // the alternative is to repeatedly get a random person from the list until we get one who is elegable.
    // in the case where the pool of people is large and there are very few people that fit our requirements,
    // it could take a lot of guessing to find someone who is elegable.
    // the negative here is it add some pre processing to make the list of qualified
    // and it adds some post processing to remove the person from the original list since we now have no idea what it's index is

    for(int i = 0; i < static_cast<int>(persons.size()); i++){

        isQualified = (canBePledge || !persons[i].IsPledge()) && (persons[i].House() == targetHouse || targetHouse == "") && (canBeEboard || !persons[i].IsEboard());
        // this condition inverted: (!canBePledge && personPtr->IsPledge()) || (mustBeNewHouse && !personPtr->CanBeBarrowedFromCommunity()) || (!canBeEboard && personPtr->IsEboard())

        if(isQualified){

            qualifiedPersons.push_back(&persons[i]);
        }
    }

    Person thePerson = *(qualifiedPersons[genRand(0, qualifiedPersons.size() - 1)]);

    int personIndex = -1;
    for(int i = 0; i < static_cast<int>(persons.size()); i++){

        if(persons[i] == thePerson){

            personIndex = i;
            break;
        }
    }

    if(static_cast<int>(qualifiedPersons.size()) == 0 || personIndex == -1){

        return Person("error", false, false, false, "");
    }

    persons.erase(persons.begin() + personIndex);
    
    return thePerson;
}

int getDayNum(const char dayChar){

    switch(dayChar){

        case 'M':{
            return 0;
            break;
        }
        case 'T':{
            return 1; 
            break;
        }
        case 'W':{
            return 2;
            break;
        }
        case 'H':{
            return 3;
            break;
        }
        case 'F':{
            return 4; 
            break;
        }
        case 'S':{
            return 5; 
            break;
        }
        case 'U':{
            return 6;   
            break;
        }
        default:{
            return -1;
            break;
        }
    }
}
