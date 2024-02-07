//Yes, these are ~450 lines of code. This has a lot of memory management issues.
//TODO: Define all variables to top || at least allocate memory for them.
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>
#include <math.h>
// Variables here to fight dynamic allocation BS.

char input[12] = "verb";
int diff = 5;

//Start of base structs.
typedef struct{
    //Base stats. 0STR, 1AGI, 2DEX, 3CON, 4PER. Might be extended.
    int str; //Physical strength.
    int agi; //Mobility, higher stats mean more energy and chances to move.
    int dex; //Speed of reactions like parrying.
    int con; //State of the character's health.
    int per; //Sharpness of senses.
    int max_health;
    int health; // Placeholder. Needs to be set once CON is defined. (CON * 10 or something like that)
    int xp;
    int coins;
    int effects[5];
    int effect_durations[5]; //Measured in turns.
    int effect_stacks[5];
    //TODO - There's bound to be more to this.
} character; //TODO - Add more stats.
typedef struct{   // Unintentional yet beautiful comments.
    char *size;  // Size of the character. Larger means more strength and parrying, smaller means more agility.
    int max_health; // Maximum health of the character.
    int health; // Set directly.
    int power; // Damage of basic hit.
    int dex;  // Chance of reactions.
    char *special[5]; //Special abilities. Go crazy.
    float specialization[5]; //Chances of triggering special abilities. Range of 0 to 1.
    int special_turns; //Number of special abilities that can be triggered per turn.
    char* name;
    int effects[5];
    int effect_durations[5]; //Measured in turns.
    int effect_stacks[5];
    char perks[3][20]; //Perks that the character has.
} npc;
/* + About NPC stats:
    Size - 5 different values: Miniscule, Small, Humanoid, Tall, Large
    Special abilities - Keep it simple, stupid. Shorten until you can't. Remember to add redundant little things here. Empty ability slots are "None".
    Specialization - Remember to sync indexes with special abilities. Easy and efficient.
*/
/* + About perks:
    Fast hands - Makes an attack after exhausting special options. Does not attack if already did in this turn.

*/
// Start of bestiary. All enemies should be defined here.
npc dummy; //Debug enemy.
npc rat;
npc rabid_rat;
npc desperate_wolf; //Weaker than a normal wolf.

void note(char *str){ // log() is taken in <math.h>
    FILE *file_ptr;
    file_ptr = fopen("logs/logs.txt", "a");

    if (file_ptr == NULL){
        printf("Can't open the log file. Please do not tamper with it.\n");
        exit(1);
    }
    time_t now = time(0);
    struct tm *ltm = localtime(&now);
    char buffer[80];
    strftime(buffer, sizeof(buffer), "%c", ltm);
    char msg[120];
    strcpy(msg, buffer);
    strcat(msg, " ");
    strcat(msg, str);
    strcat(msg, "\n");
    fputs(msg, file_ptr);
    fclose(file_ptr);
}
void setupBeasts(){
    //Dummy
    dummy.size = "Humanoid";
    dummy.max_health = 100;
    dummy.health = 100;
    dummy.power = 0;
    dummy.dex = 0;
    dummy.special[0] = "Execute"; // Eliminates the player.
    dummy.specialization[0] = 0;
    dummy.special[1] = "Self-Destruct"; // Kills the dummy.
    dummy.specialization[1] = 0;
    dummy.special[2] = "Big punch"; //Forces a dex check.
    dummy.specialization[2] = 0;
    dummy.special[3] = "Reset"; //Resets health to max health.
    dummy.specialization[3] = 1;
    dummy.special[4] = "Ignite";
    dummy.specialization[4] = 0;
    dummy.special_turns = 1;
    dummy.name = "Dummy";
    strcpy(dummy.perks[0], "Fast hands");

    //Rat
    rat.size = "Miniscule";
    rat.max_health = 10;
    rat.health = 10; //  Assuming STR = damage & 2HKO.
    rat.power = 2;  // Weak. If you die like this, find your brain.
    rat.dex = 0.2; // Mostly due to size. No parrying.
    rat.special[0] = "Screech"; //Redundant but interesting
    rat.specialization[0] = 0.4; //Not too often.
    rat.special[1] = "Bite and Cling"; // Inescapable self damage to 2x the damage.
    rat.specialization[1] = 0.2; //Rarer.
    rat.special_turns = 1;
    for(int i = 2; i < 5; i++){ //Mark rest as empty.
        rat.special[i] = "None";
        rat.specialization[i] = 0;
    }
    rat.name = "Rat";
    //Rabid rat
    rabid_rat.size = "Miniscule";
    rabid_rat.max_health = 7;
    rabid_rat.health = 7;   // Less, due to rabies.
    rabid_rat.power = 4;   // Rabies causes more aggression.
    rabid_rat.dex = 0.15; // Again, rabies.
    rabid_rat.special[0] = "Rabid bite"; // TODO - Poison attack or some bonus effect. 1.5x damage.
    rabid_rat.specialization[0] = 0.2;
    rabid_rat.special[1] = "Bite and Cling"; // TODO - Bonus damage that forces inescapable damage on user. 1.5x damage.
    rabid_rat.specialization[1] = 0.2;
    rabid_rat.special[2] = "Screech"; // Does nothing.
    rabid_rat.specialization[2] = 0.2;
    rabid_rat.special_turns = 1;
    for (int i = 3; i < 5; i++) {
        rabid_rat.special[i] = "None";
        rabid_rat.specialization[i] = 0;
    }
    rabid_rat.name = "Rabid rat";
    //TODO - Desperate wolf
    note("Beasts have been set up!");
}
character player;
void setupPlayer(){ // TODO - Add character customization.
    player.str = 5;
    player.agi = 5;
    player.dex = 5;
    player.con = 5;
    player.per = 5;
    player.max_health = player.con * 10; //50
    for(int i = 0; i < 5; i++){
        /*player.effects[i][0] = 'N';
        player.effects[i][1] = 'o';
        player.effects[i][2] = 'n';
        player.effects[i][3] = 'e'; */

        player.effects[i] = 0;

    }
    note("Player has been set up!");
}
typedef struct {
    int coin;
    int xp;
    int health; //For really special cases. Possibly redundant.
} rewards;
bool statCheck(char* stat, int dc, int adv){
    int check;
    if(strcmp(stat, "STR") == 0){
        check = player.str * 2;
        printf("You roll a strength check. Strength = %d\n", check);
    } else if (strcmp(stat, "AGI") == 0){
        check = player.agi * 2;
        printf("You roll an agility check. Agility = %d\n", check);
    } else if (strcmp(stat, "DEX") == 0){
        check = player.dex * 2;
        printf("You roll a dexterity check. Dexterity = %d\n", check);
    } else if (strcmp(stat, "CON") == 0){
        check = player.con * 2;
        printf("You roll a constitution check. Constitution = %d\n", check);
    } else if (strcmp(stat, "PER") == 0){
        check = player.per * 2;
        printf("You roll a perception check. Perception = %d\n", check);
    }
    int roll;
    check *= 2;
    roll = rand() % 20;
    bool output = roll >= (round(dc * adv));
    output? printf("You have passed the check.\n") : printf("You have failed the check.\n");
    return output;
}
void addEffect(int effect, int duration, int stacks, char target[20]){
    /* + Effects and their IDs.
    ID 0 = None. Does nothing.
    ID 1 = Burning. Deals magical damage to the target equal to the amount of stacks.
    That's it for now.
    */
    if(strcmp(target, "player") == 0){
        for(int i = 0; i < 5; i++){
            if(player.effects[i] == 1){
                player.effects[i] = effect;
                player.effect_durations[i] = duration;
                player.effect_stacks[i] = stacks;
            }
        }
    } else if(strcmp(target, "dummy") == 0){
        for(int i = 0; i < 5; i++){
            if(dummy.effects[i] == 1){
                dummy.effects[i] = effect;
                dummy.effect_durations[i] = duration;
                dummy.effect_stacks[i] = stacks;
                break;
            }
        }
    }
}
void enterCombat(npc enemy){
    char msg[32] = "Entered combat versus ";
    strcat(msg, enemy.name);
    note(msg);
    char *options[5];
    char *message;
    float special_chance = 0.0;
    float special_bp[5]; //Special breakpoints.
    special_bp[0] = 0.0;
    rewards reward;
    if (enemy.name == "Rat"){ //TODO - Special interactions should be in a list of names here. Otherwise, yeet this segment outta here.
        options[0] = "Attack";
        options[1] = "Enter stance";
        options[2] = "Disengage";
        options[3] = "Nothing";
        for(int i = 1; i < 5; i++){
            special_bp[i] = special_chance;
            special_chance += enemy.specialization[i];
        }
        reward.coin = rand() % 3;
        reward.xp = 5;
        reward.health = 0;
        message = "What are you going to do - [Attack], [Enter_stance], [Disengage], or [Nothing]?\n";
    } else if (enemy.name == "Dummy"){
        options[0] = "Attack";
        options[1] = "Enter_stance";
        options[2] = "Disengage";
        options[3] = "Nothing";
        options[4] = "Command";
    }
    enemy.health = enemy.max_health;
    while(true){
        float dmg_mult = 1;
        float pain_mult = 1;
        //PLAYER TURN
        printf("Your opponent has %d health and you have %d health.\n", enemy.health, player.health);
        for(int i = 0; i < 5; i++){
            if(player.effects[i] == 1){
            }
        }
        printf("%s", message);
        scanf("%s", input);
        char fix = input[0]; //For some reason the first letter of input keeps getting overwritten as [NULL]
        char strength;
        itoa(player.str, &strength, 10);
        input[0] = fix;
        if (strcmp(input, "Attack") == 0){
            char why[23] = "Player dealt ";
            double damage = round(((double)player.str * dmg_mult) * 10) / 10;
            sprintf(why, "%f damage.", damage); //Assuming STR = damage.
            note(why);
            enemy.health -= player.str;
        } else if (strcmp(input, "Enter_stance") == 0){
            pain_mult -= 0.2;
            note("Player entered defensive stance.");
        } else if (strcmp(input, "Disengage") == 0){
            printf("As you try to run, you realise that there is nowhere to run.(Not yet implemented)\n"); //TODO - Needs navigation.
            note("Player attempted to disengage.");
        } else if (strcmp(input, "Nothing") == 0){
            printf("You begin to observe the opponent.\n");
            note("Player does nothing.");
        } else if(strcmp(input, options[4]) == 0 && enemy.name == "Dummy"){
            printf("What are you going to command the dummy to do?\n");
            scanf("%s", input);
            int command = 20;
            for(int i = 0; i < 5; i++){
                if(strcmp(input, options[i]) == 0){
                    command = i;
                    break;
                }
            }
            if(command == 20){
                printf("Invalid command.\n");
            } else {
                printf("You command the dummy to do %s.\n", options[command]);
            }
        } else{
            printf("You indecisively stumble forward.\n");
            input[12] = '\0';
            char stupidity[34];
            strcpy(stupidity, "Player attempts to ");
            strcat(stupidity, input);
            note(stupidity);
        }
        //ENEMY TURN
        float action = rand() % 100;
        action /= 100;
        int act  = 6;
        int specials = enemy.special_turns;
        for(int i = 0; i < 5; i++){
            if(enemy.special[i] == "None" || specials == 0){
                continue;
            }
            if (action >= special_bp[i]){
                act = i;
                specials--;
            }
        }
        if(strcmp(enemy.name, "Rat") == 0){
            switch (act){
                case 0:
                printf("The rat screeches at you, expecting a result.\n");
                printf("There is no result.\n");
                break;
                case 1:
                printf("The rat pounces at you!\n");
                if (statCheck("DEX", 20, 1)){
                    printf("You caught the rat mid-pounce and swiftly throw it to the ground.\n");
                    enemy.health -= round(player.str / 2);
                } else {
                    printf("The rat bites you hard. You try to rip it off and accidentally drop it.\n");
                    player.health -= round(enemy.power * 1.5 * pain_mult);
                }
                break;
                default: //Regular attack.
                printf("The rat scratches you.\n");
                player.health -= round(enemy.power * pain_mult);
                break;
            }
        } else if (strcmp(enemy.name, "Dummy") == 0){
            switch (act){
                case 0: //Execute
                printf("The dummy calls upon dark forces to cast a magical blade.\n");
                printf("As it slashes you, you lose consciousness.\n");
                player.health = 0;
                break;
                case 1:
                printf("The dummy begins to glow.\n");
                printf("Whoa! It blew up!\n");
                enemy.health = 0;
                break;
                case 2: //Big punch
                printf("The dummy prepares for a big punch.\n");
                printf("It proceeds to punch you! Watch out!\n");
                if(statCheck("DEX", 10, 1)){
                    printf("You step to the side as the dummy falls, incapable of regaining balance.\n");
                    printf("Being an honourable fighter, you help the dummy get back up.\n");
                } else {
                    printf("You receive a powerful blow to the face!\n");
                    player.health -= round(enemy.power * 1.5 * pain_mult);
                }
                case 3:
                printf("The dummy blankly stares at you.\n");
                printf("It seems like its wounds are now gone.\n");
                dummy.health = dummy.max_health;
                break;
                case 4: //Ignite
                printf("The dummy casts [Ignite]!\n");
                printf("It appears that you are now on fire.\n");
                addEffect(1, 3, 3, "player");
            }
        }
        //EFFECT TURN - At the end of a turn. I know that's weird.
        //char ex_effects[10][20]; //Unused for now.
        for(int i = 0; i < 5; i++){
            if(player.effects[i] == 0){
                continue;
            } else if(player.effects[i] == 1){
                player.health -= player.effect_stacks[i]; //TODO - Add effects to setupPlayer()
                player.effect_stacks[i]--;
                if(player.effect_stacks[i] == 0){
                    player.effects[i] = 0;
                }
            }
        }
        for(int i = 0; i < 5; i++){
            if(enemy.effects[i] == 0){
                continue;
            } else if (enemy.effects[i] == 1){
                enemy.health -= enemy.effect_stacks[i];
                enemy.effect_stacks[i]--;
                if(enemy.effect_stacks[i] == 0){
                    enemy.effects[i] = 0;
                }
            }
        }
        //DEATH CHECK
        if(player.health <= 0){
            printf("You have died.\n");
            break;
        } else if(enemy.health <= 0){
            printf("You are victorious, the enemy has died!\n");
            break;
        }

    }
    int newHealth = player.health + reward.health > player.max_health ? player.max_health : reward.health + reward.health;
    reward.health = newHealth;
    player.coins += reward.coin;
    player.xp += reward.xp;
    printf("You have earned %d coins and %d experience points.\n", reward.coin, reward.xp);
}
int chooseDifficulty(char *arg){
    if (strcmp(arg, "help") == 0) {
        printf("There are 4 difficulty levels:\nEasy - You are a hardened soldier ready to dominate the world with raw strength.\nMedium - You are a soldier. Ready enough to take on the beasts that lurk ahead.\nHard - You are a mere peasant. Hope is your best weapon.\nInsane - You are not just a peasant. You are cursed.\n");
        note("Help offered.");
        return 6;
    } else if (strcmp(arg, "Easy") == 0 || strcmp(arg, "easy") == 0) {
        return 1;
    } else if (strcmp(arg, "Medium") == 0 || strcmp(arg, "medium") == 0) {
        return 2;
    } else if (strcmp(arg, "Hard") == 0 || strcmp(arg, "hard") == 0) {
        return 3;
    } else if (strcmp(arg, "Insane") == 0 || strcmp(arg, "insane") == 0) {
        return 4;
    } else if (strcmp(arg, "Debug") == 0) {
        char ans[7] = "error";
        scanf("%s", ans);
        if (strcmp(ans, "pass123") == 0) {
            return 5;
        }
        printf("Don't.");
        note("User attempted to enter debug mode.");
        return 6;
    } else {
        printf("This is not a valid difficulty setting.\n");
        note(strcat("User tried to use difficulty setting ", arg));
        return 6;
    }
}
void intro(int diff){
    char input[6] = "error";
    printf("In an alternate world, a two-faced volunteer betrays the sadistic god of wisdom. Thrusting a small dagger towards its neck, only to watch the god transform into a small cloud. As a swift punishment, they are sent to the purgatory of warriors.\n\n");
    if(diff == 6){
        return;
    }
    switch(diff){
        case 1: //Easy
        printf("You have landed in an alien world. You get up, brush off your armour and feel embarrassed for the failed attack. Your sword could\'ve been a better option.");
        printf("Taking a swift glance of the small clearing, you notice a small beast run away.");
        //setupPlayer();
        //TODO - Begin navigation.
        break;
        case 2: //Medium
        printf("You fall in a small clearing. After rethinking about what\'s happened, you feel like you might not wish to go back.\n");
        printf("You notice a small beast get close to you. That's no beast. That's just a big rat.\n");
        //setupPlayer();
        player.health = player.max_health;
        enterCombat(rat);
        break;
        case 3: //Hard
        printf("As hit the ground spraining your ankle, dangerous thoughts corrupt you. What a failure. The wisdom the god knew, he just keeps for himself. Get back at him, in the name of humanity.");
        printf("A large rat approaches you. Not only does it get up on its hind legs, something seems off.");
        //setupPlayer();

        //TODO - Add PER(ception) check & Enter combat vs rabid rat.
        case 4: //Insane
        printf("You, an ostracised cultist of Melissa, the goddess of mad rage, hit the ground hard. Malnourished and weak, you must press onward. Beat his stupid game.");
        printf("A rat pounces you. Will you [catch] or [dodge] it?");
        scanf("%s", input);
        //TODO - Add DEX check & Disadvantage rolls. (For catching, success damages the rat with a throw.)
        // + Also the rat will be caught and ripped apart by a wolf.
        case 5:
        printf("Hello. Starting combat debug.\n");
        enterCombat(dummy);
        default:
        printf("Something went terribly wrong.\n");
    }
}
int main(int argc, char *argv[]){
    //fopen("logs/logs.txt" 'w');
    note("Log file has been opened!");
    setupBeasts();
    setupPlayer();
    printf("Input your desired difficulty level - [Easy], [Medium], [Hard], [Insane]\n");
    strcpy(input, "error");
    scanf("%s", input);
    //DONE: diff moved to top.
    diff = chooseDifficulty(input);
    char why[23] = "why"; //My actual reaction.
    printf("%s\n%d\n", why, diff); //TODO - Delete this.
    sprintf(why, "Difficulty chosen - %d\n", diff); //diff becomes 0?????
    note(why);
    if (diff == 6){
        return 1;
    }
    intro(diff); //And now diff is 327**. what the actual fuck.
    note("End of script reached!");
    return 0;
}
//C:\Users\SKOLENS\Documents\Programmesana\Ernests_Gulbis_10i\git_adv\text_rpg\adv.c