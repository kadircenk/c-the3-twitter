#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/***** STRUCT DEFINITIONS *****/
typedef struct graph_node{
    int id;
    struct graph_node *next;
}Follower_node, *FollowerPtr;

typedef struct tweet_c{
    int msg_id;
    int author_id;
    struct tweet_c *next;
}Msg, *MsgPtr;

typedef struct person{
    int id;
    char name[10];
    int isActive;
    struct person *next;
    FollowerPtr followers;
    MsgPtr messages;
}User, *UserList;

/**** GLOBAL VARIABLES *****/
UserList ulist = NULL;
int msg_id = 1, user_id = 1;
FILE * filei, *fileo;

/**** FUNCTION DECLARATIONS ****/

// Add a new user with her name and id.
// It is active by default
void add_user(char *username, int *id){
    UserList newU = (UserList)malloc(sizeof(User));
    newU->id = *id;
    newU->isActive = 1;
    strcpy(newU->name, username);
    (*id)++;

UserList temp = ulist;

    if (!ulist)
        ulist = newU;
    else{
        while(temp->next)  temp=temp->next;
        temp->next = newU;
    }
}

// get the user reference of a user by her name
User * find_user_by_name(char *name){
    UserList temp = ulist;
for(;temp;temp = temp->next)
    if ( !strcmp(temp->name, name))
        return temp;
}

// get the user reference of a user by her id
User * find_user_by_id(int id){
    UserList temp = ulist;
for(;temp;temp = temp->next)
    if (temp->id == id)
        return temp;
}

// Add the follower user to the follower list of a followed user
void follow(char *follower, char *followed){
    User *user1 = find_user_by_name(followed);
    User *user2 = find_user_by_name(follower);

    FollowerPtr f = (FollowerPtr)malloc(sizeof(Follower_node));
    f->id = user2->id;  
    f->next = NULL;

    FollowerPtr temp = user1->followers;
    if (!user1->followers)
        user1->followers = f;
    else
    {
        while(temp->next) temp = temp->next;
        temp->next = f;
    }
}

// Delete the messages from a user by a certain author id
void delete_messages(User *user, int author_id){
    if(!user->messages)	return;
	
    MsgPtr p = user->messages;  
    MsgPtr q = p->next;

    while(p && p->author_id == author_id){ // while it is in the first message
        user->messages = user->messages->next;
        free(p);
        p = user->messages;
    }
    while(q){
        if (q->author_id == author_id){ // check
            p->next = q->next;
            free(q);
            q = p->next;
        }
        else{ // pass the next
            p = q;
            q = q->next;
        }
    }
}

// Delete follower from the follower list of a followed user
// Also delete the messages belong to followed person
void unfollow(char *follower, char *followed){
    User *user1 = find_user_by_name(followed);
    User *user2 = find_user_by_name(follower);
    int delete = user2->id;

    FollowerPtr p = user1->followers;
    FollowerPtr q = p->next;

    // 1. First delete from the followers of a followed person
    if (p->id == delete){
        free(p);
        user1->followers = user1->followers->next;
    }

    while(q){
        if (q->id == delete){
            p->next = q->next;
            free(q);
        }
        else{
            p = q;
            q = q->next;
        }
    }

    // 2. Delete the messages belong to followed person
    delete_messages(user2, user1->id);
}

void put_message(User * user, MsgPtr msg){

    MsgPtr newMsg = (MsgPtr)malloc(sizeof(Msg));
    newMsg->msg_id = msg->msg_id;
    newMsg->author_id = msg->author_id;
    newMsg->next = NULL;

    MsgPtr temp = user->messages;
    MsgPtr ptemp = NULL;

    if (!user->messages) // It is the first message
        user->messages = newMsg;
    else{
        // Find the exact location for a new message (temp)
        while(temp && (newMsg->msg_id > temp->msg_id)) {
            ptemp = temp;
            temp = temp->next;
        }
        if (!ptemp){ // If new message (temp) will be first node
            user->messages = newMsg;
            newMsg->next = temp;
        }
        else{ // If temp will be inter-node
            ptemp->next = newMsg;
            newMsg->next = temp;
        }
    }
}

// It send a message to the sender first, then to active followers
void send(char *sender, int *msg_id){
    User *user = find_user_by_name(sender);
    MsgPtr msg = (MsgPtr)malloc(sizeof(Msg));
    msg->msg_id = *msg_id;
    msg->author_id = user->id;

    put_message(user, msg); // Message it user's itself

    FollowerPtr ftemp = user->followers;
    for(;ftemp;ftemp = ftemp->next) // Message to followers
    {
      User *temp_user = find_user_by_id(ftemp->id);
      if (temp_user->isActive){
          put_message(temp_user, msg); // Message it to active followers
      }
    }
    (*msg_id)++;
}

// Get the length of the message list of a user
// It takes reference to message list
int message_length(MsgPtr mtemp){
    int counter = 0;
    for(;mtemp;mtemp = mtemp->next)
        counter++;

return counter;
}

// Get kth last message of a user
// It takes reference the user and integer k
int get_message_by_k(User *user, int k){
    MsgPtr msgp = user->messages;
    int msgNo = message_length(msgp) - k + 1;
    int i = 1;
    for(; i < msgNo; msgp = msgp->next, i++);

    return msgp->msg_id;
}

// Resend kth last message of user by getting her name
void re_send(char *sender, int k){
    User *user = find_user_by_name(sender);
    MsgPtr msg = (MsgPtr)malloc(sizeof(Msg));

    msg->msg_id = get_message_by_k(user, k);
    msg->author_id = user->id;

    put_message(user, msg); // Message it user's itself

    FollowerPtr ftemp = user->followers;
    for(;ftemp;ftemp = ftemp->next) // Message to followers
    {
      User *temp_user = find_user_by_id(ftemp->id);
      if (temp_user->isActive){ // check if she is active
          put_message(temp_user, msg); // Message it to active followers
      }
    }
}

// Inactivate a user by name
void inactive(char *username){
    User *user = find_user_by_name(username);
    user->isActive = 0;
}

// Activate a user by name
void active(char *username){
    User *user = find_user_by_name(username);
    user->isActive = 1;
}

// It shows in detail (followers, authors, messages etc.)
void show_messages2(UserList list){
    for(;list;list = list->next)
    {
        printf("---------------------\n");
        printf("username: %s, id: %d, active:%d\n", list->name, list->id, list->isActive);
        FollowerPtr ftemp = list->followers;
        printf("Followers: ");
        for(;ftemp;ftemp = ftemp->next)
            printf("%d ", ftemp->id);
            printf("\n");
        MsgPtr mtemp = list->messages;
        for(;mtemp;mtemp = mtemp->next)
            printf("Message id: %d  Author: %d\n", mtemp->msg_id, mtemp->author_id);
    }
    printf("====  END  ====\n");
}

// Recursive printf to print the linked list from end to start
void myPrintf(MsgPtr mtemp){
    if (!mtemp)
        return;
    myPrintf(mtemp->next);
    printf(" %d", mtemp->msg_id);
    fprintf(fileo, " %d", mtemp->msg_id);
}

void show_messages(UserList list){
    for(;list;list = list->next)
    {
        printf("%d:", list->id);
        fprintf(fileo, "%d:", list->id);

        myPrintf(list->messages);
        printf("\n");
        fprintf(fileo, "\n");
    }
}

void read_from_file(){

    char cmd[20];
    char str1[20];
    char str2[20];
    int k;
    char c;

if (filei) {
        while (fscanf(filei, "%s", cmd)!=EOF){
            if (!strcmp(cmd,"ADD")){
                fscanf(filei, "%s", str1);
                add_user(str1, &user_id);
            }
            else if(!strcmp(cmd,"SEND")){
                fscanf(filei, "%s", str1);
                send(str1, &msg_id);
            }
            else if(!strcmp(cmd,"FOLLOW")){
                fscanf(filei, "%s", str1);
                fscanf(filei, "%s", str2);
                follow(str1, str2);
            }
            else if(!strcmp(cmd,"RESEND")){
                fscanf(filei, "%s", str1);
                fscanf(filei, "%d", &k);
                re_send(str1, k);
            }
            else if(!strcmp(cmd,"UNFOLLOW")){
                fscanf(filei, "%s", str1);
                fscanf(filei, "%s", str2);
                unfollow(str1, str2);
            }
            else if (!strcmp(cmd,"ACTIVE")){
                fscanf(filei, "%s", str1);
                active(str1);
            }
            else if (!strcmp(cmd,"INACTIVE")){
                fscanf(filei, "%s", str1);
                inactive(str1);
            }
        }
        fclose(filei);
    }
    else
        printf("Input file does not exist!");
}

save_to_file(){
    show_messages(ulist);
    fclose(fileo);
}

int main(int argc, char *argv[])
{
    fileo = fopen("output.txt", "w");
    filei = fopen("input.txt", "r");
	
    read_from_file();
    save_to_file();

    return 0;
}
