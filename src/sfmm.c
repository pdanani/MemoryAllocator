/**
 * All functions you make for the assignment must be implemented in this file.
 * Do not submit your assignment with a main function in this file.
 * If you submit with a main function in this file, you will get a zero.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#include <errno.h> //must include to edit errno

#include "debug.h"
#include "sfmm.h"

#define MROW 8
#define ALIGN 16
#define BLOCK 32

static int initialized=0;

void *matchChecker(sf_block* a, int v, int x);

void coalesce(void* pp){ //remember to remove off free list
    //check pre allocated and next allocate

    sf_block *current_block=(sf_block*)pp;
    int deciphered=current_block->prev_footer^sf_magic();//get deciphered footer showing size and bits
    void* temp=(void*)current_block-(deciphered&BLOCK_SIZE_MASK);//brings us to previous struct
    sf_block *prev_block=(sf_block*)temp;
    temp=(void*) current_block+(current_block->header&BLOCK_SIZE_MASK);
     sf_block*next_block=temp;
    sf_block* new_ftr=next_block;
    //both are allocated


    if(((current_block->header)&1)==1&&(next_block->header&2)==2){

    if((current_block->header&2)==2){
        current_block->header-=2;
        next_block->prev_footer=(current_block->header)^sf_magic();

    }
    if((next_block->header&1)==1){
        next_block->header-=1;
        void *tran= (void*)new_ftr+(next_block->header&BLOCK_SIZE_MASK);
        new_ftr=tran;
        new_ftr->prev_footer=next_block->header^sf_magic();

    }


    int pick =0;
         int remainder = current_block->header&BLOCK_SIZE_MASK;
            if(remainder<=32)
                pick =0;
            if(remainder>32 && remainder<=64)
                pick=1;
            if(remainder>64 && remainder<=128)
                pick=3;
            if(remainder>256 && remainder<=512)
                pick=4;
            if(remainder>512 && remainder<=1024)
                pick=5;
            if(remainder>1024 && remainder<=2048)
                pick=6;
            if(remainder>2048 && remainder<=4096)
                pick=7;
            if(remainder>4096)
                pick=8;

         current_block->body.links.next = sf_free_list_heads[pick].body.links.next;
        current_block->body.links.prev = &sf_free_list_heads[pick];
        sf_free_list_heads[pick].body.links.next->body.links.prev = current_block;
        sf_free_list_heads[pick].body.links.next = current_block;
}


    //top is empty
    if(((prev_block->header&2)==2)&&((next_block->header&2)==0)){
        //printf("%s\n", "made it case 3");
         //remove links for prev and next from free lists.
        prev_block->body.links.prev->body.links.next = prev_block->body.links.next;
        prev_block->body.links.next->body.links.prev = prev_block->body.links.prev;

//make next block prev alloc 0
        void *tran= (char*)new_ftr+(next_block->header&BLOCK_SIZE_MASK);
        new_ftr=tran;
        prev_block->header=(prev_block->header&BLOCK_SIZE_MASK)+//continues on next line
            (current_block->header&BLOCK_SIZE_MASK)+1;

        current_block->header=0;
        if((next_block->header&1)==1){
            next_block->header-=1;
            new_ftr->prev_footer=next_block->header^sf_magic();
        }

        current_block->prev_footer=0; //reset all the headeers and footers in between

        next_block->prev_footer=prev_block->header^sf_magic();



        //make sure to also do the footer of the next next block.


         int pick =0;
         int remainder = prev_block->header&BLOCK_SIZE_MASK;
            if(remainder<=32)
                pick =0;
            if(remainder>32 && remainder<=64)
                pick=1;
            if(remainder>64 && remainder<=128)
                pick=3;
            if(remainder>256 && remainder<=512)
                pick=4;
            if(remainder>512 && remainder<=1024)
                pick=5;
            if(remainder>1024 && remainder<=2048)
                pick=6;
            if(remainder>2048 && remainder<=4096)
                pick=7;
            if(remainder>4096)
                pick=8;


        prev_block->body.links.next = sf_free_list_heads[pick].body.links.next;
        prev_block->body.links.prev = &sf_free_list_heads[pick];
        sf_free_list_heads[pick].body.links.next->body.links.prev = prev_block;
        sf_free_list_heads[pick].body.links.next = prev_block;


    }

    //bottom is empty
    if(((prev_block->header&2)==2)&&(next_block->header&2)==0){


         //remove links for prev and next from free lists.
        next_block->body.links.prev->body.links.next = next_block->body.links.next;
        next_block->body.links.next->body.links.prev = next_block->body.links.prev;

//make next block prev alloc 0
        void *tran= (void*)new_ftr+(next_block->header&BLOCK_SIZE_MASK);
        new_ftr=tran;
        current_block->header=(current_block->header&BLOCK_SIZE_MASK)+//continues on next line
            (next_block->header&BLOCK_SIZE_MASK)+1;

         next_block->header=0;

        next_block->prev_footer=0;



        //make sure to also do the footer of the next next block.
        new_ftr->prev_footer=current_block->header^sf_magic();



         int pick =0;
         int remainder = current_block->header&BLOCK_SIZE_MASK;
            if(remainder<=32)
                pick =0;
            if(remainder>32 && remainder<=64)
                pick=1;
            if(remainder>64 && remainder<=128)
                pick=3;
            if(remainder>256 && remainder<=512)
                pick=4;
            if(remainder>512 && remainder<=1024)
                pick=5;
            if(remainder>1024 && remainder<=2048)
                pick=6;
            if(remainder>2048 && remainder<=4096)
                pick=7;
            if(remainder>4096)
                pick=8;


        current_block->body.links.next = sf_free_list_heads[pick].body.links.next;
        current_block->body.links.prev = &sf_free_list_heads[pick];
        sf_free_list_heads[pick].body.links.next->body.links.prev = current_block;
        sf_free_list_heads[pick].body.links.next = current_block;
    }

    //both are empty
    if((current_block->header&1)==0&&(next_block->header&2)==0){
        //remove links for prev and next from free lists.
        prev_block->body.links.prev->body.links.next = prev_block->body.links.next;
        prev_block->body.links.next->body.links.prev = prev_block->body.links.prev;

        next_block->body.links.prev->body.links.next = next_block->body.links.next;
        next_block->body.links.next->body.links.prev = next_block->body.links.prev;



        void *tran= (void*)new_ftr+(next_block->header&BLOCK_SIZE_MASK);
        new_ftr=tran;
        prev_block->header=(prev_block->header&BLOCK_SIZE_MASK)+//continues on next line
            (current_block->header&BLOCK_SIZE_MASK)+(next_block->header&BLOCK_SIZE_MASK)+1;

        current_block->header=0;
        next_block->header=0;
        next_block->prev_footer=0;
        current_block->prev_footer=0; //reset all the headeers and footers in between

        //make sure to also do the footer of the next next block.
        new_ftr->prev_footer=prev_block->header^sf_magic();



         int pick =0;
         int remainder = prev_block->header&BLOCK_SIZE_MASK;
            if(remainder<=32)
                pick =0;
            if(remainder>32 && remainder<=64)
                pick=1;
            if(remainder>64 && remainder<=128)
                pick=3;
            if(remainder>256 && remainder<=512)
                pick=4;
            if(remainder>512 && remainder<=1024)
                pick=5;
            if(remainder>1024 && remainder<=2048)
                pick=6;
            if(remainder>2048 && remainder<=4096)
                pick=7;
            if(remainder>4096)
                pick=8;


        prev_block->body.links.next = sf_free_list_heads[pick].body.links.next;
        prev_block->body.links.prev = &sf_free_list_heads[pick];

        sf_free_list_heads[pick].body.links.next->body.links.prev = prev_block;
        sf_free_list_heads[pick].body.links.next = prev_block;

//now remove stuff from free lists and insert new bigger free block into free lsit.
    }








}

void* splitA(void* p, int remainder, int block_size){
             sf_block *split_alloc= (sf_block*)p;
            void *partition = (void *)p + block_size;
            sf_block *split_free = (sf_block*)partition;
//free +1
            void *partition2=(void*)split_free + remainder;

            sf_block *free_ftr= (sf_block*)partition2;
            //split+3

            split_free->header=remainder+1;//add allocated bit.

            if((split_alloc->header&1)==1)
                split_alloc->header= block_size+2+1;
            else{
                split_alloc->header=block_size+2;//allocated bit set to 1
            }
            split_free->prev_footer=split_alloc->header^sf_magic();
//next block if split_alloc is always 0 for prev allocated because of the split free on bottom
            //check
            //split_free+=remainder;//go to next struct to get ftr
            free_ftr->prev_footer=(remainder+1)^sf_magic();//make footer alloc bit =1
            //split_free-=remainder;
            return split_alloc;

}
void* splitF(void* p, int remainder, int block_size){
             sf_block *split_alloc= (sf_block*)p;
            void *partition = (void *)p + block_size;
            sf_block *split_free = (sf_block*)partition;
//free +1
            void *partition2=(void*)split_free + remainder;

            sf_block *free_ftr= (sf_block*)partition2;
            //split+3

            split_free->header=remainder+1;//add allocated bit.

            if((split_alloc->header&1)==1)
                split_alloc->header= block_size+2+1;
            else{
                split_alloc->header=block_size+2;//allocated bit set to 1
            }
            split_free->prev_footer=split_alloc->header^sf_magic();
//next block if split_alloc is always 0 for prev allocated because of the split free on bottom
            //check
            //split_free+=remainder;//go to next struct to get ftr
            free_ftr->prev_footer=(remainder+1)^sf_magic();//make footer alloc bit =1
            //split_free-=remainder;
            return split_free;

}
void *sf_malloc(size_t size) {

    if(size==0)
        return NULL;

    if (initialized==0){

        void *page_ptr=sf_mem_grow(); //start a new page and set the pointer to
        // the beginning of the page to pg ptr
        if(page_ptr==NULL){
            sf_errno=ENOMEM;//if page failed, return NULL
            return NULL;
        }


        //now set thr prologue in the first ever page
         sf_prologue *prologue_ptr = (sf_prologue *)page_ptr;
         prologue_ptr->header=35;
      //  prologue_ptr->unused1=0;
        //prologue_ptr->unused2=0;
        prologue_ptr->footer=35^sf_magic();


/*The remainder of the memory in this first page should then be inserted into
the free list as a single block.*/
         //initalize the free list head sentinals
        for(int i=0;i<NUM_FREE_LISTS;i++){
            sf_free_list_heads[i].body.links.next=&sf_free_list_heads[i];
            sf_free_list_heads[i].body.links.prev=&sf_free_list_heads[i];

        }

        int remainder=4096-48;//keep the reaminder of remaining space inside a temporary variable
        sf_block *first_block = (sf_block *) (page_ptr+ 32);
         first_block->header = remainder+1 ;//makes it allocated with the prev bit!!!
//
       first_block->body.links.next=first_block;//there is only one free block, make it point to itself.
       first_block->body.links.prev=first_block;



  // Set the links for new free block:
        first_block->body.links.next = sf_free_list_heads[7].body.links.next;
        first_block->body.links.prev = &sf_free_list_heads[7];
        sf_free_list_heads[7].body.links.next = first_block;
        first_block->body.links.next->body.links.prev = first_block;


        sf_block *ftr_ptr=(sf_block*)(sf_mem_end()-16);
        ftr_ptr->prev_footer=(remainder+1)^sf_magic();
        ftr_ptr->header=2;
        //header is aligned with epilogue header
        //now set footer of first free bloc
        initialized=1;


    }

    //now allocate for first block
    int pre_pad= size+16;//add header footer
    int padding=0;
    if(pre_pad%16!=0)
        padding= 16-(pre_pad%16);
    int block_size=pre_pad+padding;//add 16 for header and footer and padding if necessary
    //now that you have the total block size go ahead and check free list for available  block
    int M=32;
    int valid =0;

//choose the right index

   // sf_block *free_ptr= sf_free_list_heads[choice].body.links.next;//go to first free list in index
     sf_block *chosen_ptr;

     for(int i=0;i<NUM_FREE_LISTS;i++){
        sf_block *free_ptr= sf_free_list_heads[i].body.links.next;//go to first free list in index

        if(block_size==32&&i==0){
            if(matchChecker(free_ptr,i,size)==0){
                M*=2;
                continue;

            }
            chosen_ptr=(sf_block*)(matchChecker(free_ptr,i,size));
            valid=1;
            break;
        }


        else if(block_size<=(M) && sf_free_list_heads[i].body.links.next!=&sf_free_list_heads[i]){
            if(matchChecker(free_ptr,i,size)==0){
                M*=2;
                continue;//if 0 then find next largest.
            }
            chosen_ptr=(sf_block*)(matchChecker(free_ptr,i,size));
            valid=1;
            break;
        }
        M*=2;//this loops helps us find the  right index and makes sure its not empty.

    }




//if found a valid block then
    if(valid==1){
        //remove the free list links.
        int extra = (chosen_ptr->header&BLOCK_SIZE_MASK)-block_size;

        chosen_ptr->body.links.prev->body.links.next = chosen_ptr->body.links.next;
        chosen_ptr->body.links.next->body.links.prev = chosen_ptr->body.links.prev;


        if(extra>=32){//remainder
           sf_block *split_alloc= chosen_ptr;
           split_alloc=(sf_block*) splitA(split_alloc,extra,block_size );
      sf_block* split_free=(sf_block*)splitF(split_alloc,extra,block_size );
        //tags


            int pick =0;
            if(extra<=32)
                pick =0;
            if(extra>32 && extra<=64)
                pick=1;
            if(extra>64 && extra<=128)
                pick=2;
            if(extra>128 && extra<=256)
                pick=3;
            if(extra>256 && extra<=512)
                pick=4;
            if(extra>512 && extra<=1024)
                pick=5;
            if(extra>1024 && extra<=2048)
                pick=6;
            if(extra>2048 && extra<=4096)
                pick=7;
            if(extra>4096)
                pick=8;
        split_free->body.links.next = sf_free_list_heads[pick].body.links.next;
        split_free->body.links.prev = &sf_free_list_heads[pick];
        sf_free_list_heads[pick].body.links.next = split_free;
        split_free->body.links.next->body.links.prev = split_free;

      //  sf_show_block(split_free);

            return split_alloc-> body.payload;

        }
        else{
            //make header pointer block size the whole thing and add important bits and then return payload.

             chosen_ptr->body.links.prev->body.links.next = chosen_ptr->body.links.next;
             chosen_ptr->body.links.next->body.links.prev = chosen_ptr->body.links.prev;//remove from free list

            chosen_ptr->header=block_size+3;//make allocated bit =1
            //move to footer and change footer. then change prev allocated of next block...
            void *middleman= (void*)chosen_ptr+(chosen_ptr->header&BLOCK_SIZE_MASK);
            sf_block * point_foot= (sf_block*)middleman;

            //chosen_ptr+=(chosen_ptr->header&BLOCK_SIZE_MASK);
            point_foot->prev_footer=(block_size+3)^sf_magic(); //make allocated bit 1.

            point_foot->header=point_foot->header+1;//make next block prev alloc
            int briana= point_foot-> header;//holds  header of next

            if((briana&BLOCK_SIZE_MASK)!=0){//if not at epligoeue!
            middleman=(void*)middleman+((point_foot->header)&BLOCK_SIZE_MASK);//get footer of next block
            point_foot=(sf_block*) middleman;
            point_foot-> prev_footer= briana^sf_magic(); //sf _magic it
           }

            return chosen_ptr->body.payload;

        }


//readtis...

    //if there is no valid size that means you want to use a new page
    static int set=0;
    sf_block *replacement_header;
    if(valid==0){
        while(set==0){
            void *new_page = sf_mem_grow(); //pointer to beginning of new page
            if(new_page == NULL){
              sf_errno = ENOMEM;
              return NULL;
             }

            replacement_header = (sf_block *) (new_page - 16); //move back to footer right before epilogue to make it the new header of block.


            sf_block* new_ftr = (sf_block*)sf_mem_end() -16;//right before new epilogue

            int decipher= (replacement_header->prev_footer)^ sf_magic();//this removes the sf_magic.
//            sf_epilogue * new_epi = (sf_epilogue *)(sf_mem_end() - 8);//new epilogue location
            new_ftr->header=2;//the whole point of making epilogue move is because of allocation, which is always bottom oriented.
            sf_block* free_collect;
//first check previous header if allocated then make new header where epilogue is and move epilog eodnw to  sf -8
        //then split the new block.


        //else  if prev is free then coalesce, move the footer down, edit the header size,

            if((decipher&2)==2){//this means that the very last block before the old epilogue is allocated..


                new_ftr->prev_footer=(4096)^sf_magic();//size of footer right before new epi
                replacement_header->header=(4096);//prev block is allocated!!
            //change header and footer
            //now check if block size is less than block size
                if (block_size<=((replacement_header->header)&BLOCK_SIZE_MASK))
                    set=1;

            //replacement header holds the free block that starts at old epilogue
            //
            }
            else if((decipher&2)==0){//means that the block before the new page is empty...
            //now we must coalesce after calling memgrow
            //so previous block is free.... go to prev block
            //find size, go to header, move footer down to bottom of new page,
            //add size of new mem grow and previous free block
            //now check the sizes to see if its a fit
            //move on to next part (splitting)
                void* calc=(void*)replacement_header-(decipher&BLOCK_SIZE_MASK);

                free_collect=(sf_block*) calc;//from last_footer to all the way to prev struct

                replacement_header-> prev_footer=0; //remove the footer of the block.
                replacement_header->header=0;//remove epilgoue!!old one
                //bring us back to header of previous free block
                free_collect->body.links.prev->body.links.next = chosen_ptr->body.links.next;//remove rom free lsit.
                free_collect->body.links.next->body.links.prev = chosen_ptr->body.links.prev;
                free_collect->header=(free_collect->header)+4096;//add new page to last block.
                new_ftr->prev_footer=((free_collect->header))^sf_magic();

                if(block_size<=((free_collect->header) & BLOCK_SIZE_MASK))//free collect is the name of coalescecd free block
                    set=2;

            }
        }


            sf_block * replacement_header = (sf_block *) (sf_mem_end() - 16); //move back to footer right before epilogue to make it the new header of block.
            int decipher= (replacement_header->prev_footer)^ sf_magic();//this removes the sf_magic.
             void* calc=(void*)replacement_header-(decipher&BLOCK_SIZE_MASK);

                sf_block* free_collect=(sf_block*) calc;

        int remainder=0;
        if(set==1){ //that means use  the first case  replacement header struct.
              remainder =(replacement_header->header&BLOCK_SIZE_MASK)- block_size;
             free_collect=replacement_header;//make free _ collect point to the appropriate place for modularity

             //also do not forget

        }
        if(set==2){//use case where there is a coaslesced free block. free_collect ptr
             remainder =(free_collect->header&BLOCK_SIZE_MASK)- block_size;

        }

        if(remainder>=32){
//tags1
            sf_block* split_alloc=free_collect;
             split_alloc=(sf_block*)splitA(split_alloc,remainder, block_size);
            sf_block* split_free=(sf_block*)splitF(split_alloc,remainder,block_size);

            //now make sure next block also has  prev allocated bit to next block

            // now put free block in appropriate list.
            int pick =0;
            if(remainder<=32)
                pick =0;
            if(remainder>32 && remainder<=64)
                pick=1;
            if(remainder>64 && remainder<=128)
                pick=3;
            if(remainder>256 && remainder<=512)
                pick=4;
            if(remainder>512 && remainder<=1024)
                pick=5;
            if(remainder>1024 && remainder<=2048)
                pick=6;
            if(remainder>2048 && remainder<=4096)
                pick=7;
            if(remainder>4096)
                pick=8;


           // split_free->body.links.next=sf_free_list_heads[pick].body.links.next;
           // split_free->body.links.prev = &sf_free_list_heads[pick];
           // sf_free_list_heads[pick].body.links.next->body.links.prev = split_free;
            //sf_free_list_heads[pick].body.links.next = split_free;



            //link and insert
        split_free->body.links.next = sf_free_list_heads[pick].body.links.next;
        split_free->body.links.prev = &sf_free_list_heads[pick];
        sf_free_list_heads[pick].body.links.next = split_free;
        (split_free->body.links.next)->body.links.prev = split_free;



            return split_alloc-> body.payload;

        }
        else if(remainder<32){


            free_collect->header=block_size+3;//make allocated bit =1
            //move to footer and change footer. then change prev allocated of next block...
            void* tmp=(void*)free_collect+(free_collect->header&BLOCK_SIZE_MASK);
            sf_block* sf_next= (sf_block*)tmp;
            sf_next->prev_footer=(block_size+3)^sf_magic(); //make allocated bit 1.

            sf_next->header=sf_next->header+1;//make next block prev alloc
             int briana= sf_next-> header;

            if((briana&BLOCK_SIZE_MASK)!=0){//if not at epligoeue!
            tmp=(void*)sf_next+((sf_next->header)&BLOCK_SIZE_MASK);//get footer of next block
            sf_block* sf_nxt=(sf_block*)tmp;
            sf_nxt-> prev_footer= briana^sf_magic(); //sf _magic it

        }
            //allocate full block.
            //do not forget to do prev alloc is 1 because at this point you have coalesced.............
            //then insert top at frree list and link the pointers next prev.
            return free_collect->body.payload;

        }
        //now you have  a free block that first ur block_size..
        //split the block if necessary.
        //you're done with malloc.

            //sf mem_grow
    }
    set=0;

}





//simple i=choice
    //choose first linkedlist that fits in that index. FIFO and check if nothing fits to move onto next class
    //use an Sf block pointer to access the size of the free block using the header and masking
//make a fucntion that returns a boolean.. now if the boolean returns false "0" then you want to "continue " instead of breaking

//using this logic, we can use the helper function to make the
//what if it goes thru  and now
    return NULL;

}

int check_valid(void *pp){
    sf_block* p=(sf_block*)pp;
    int translate=p->prev_footer^sf_magic();

    void *x=(void*)pp-(translate&BLOCK_SIZE_MASK);
    sf_block* prv=(sf_block*)x;
     void *z=(void*)pp+(p->header&BLOCK_SIZE_MASK);
    sf_block* y=(sf_block*)z;

    if(pp==NULL)
        return 0;
    if((p->header&2)==0)
        return 0;
    if((p->header&BLOCK_SIZE_MASK)<32)
        return 0;
    if((p->header&1)==0 &&((prv->header&2)==2))
        return 0;
    if(p->header!=(y->prev_footer^sf_magic()))
        return 0;


    return 1;
}

void sf_free(void *pp) {
    //if(pp==NULL)

    pp-=16;//move to before the payload

    if( check_valid(pp)==0)
        abort();



    coalesce(pp);




    return;
}









void *sf_realloc(void *pp, size_t rsize) {
    if(check_valid(pp)==0){
        sf_errno=EINVAL;
        return NULL;
    }
    if(rsize == 0)
    {
        free(pp);

        return NULL;
    }


    sf_block *p = (sf_block *)(pp-16);


    if(rsize == 0) {
        free(pp);
        return NULL;
    }

    if(p->header < rsize)
    {
        void *x = sf_malloc(rsize);
        if(x == NULL)
            return NULL;
        sf_free(pp);
    }

    return NULL;}



void* matchChecker(sf_block* free_ptr,int choice,int size){//this only helps to show if the list were in has a good block

    while(free_ptr!=&sf_free_list_heads[choice]){//make sure to end after last node
        int pre_masked=free_ptr-> header;//now we have the header size + alloc bits in a temp var
        if (size>(pre_masked&BLOCK_SIZE_MASK)){
            free_ptr=free_ptr->body.links.next; //check next free block in the index
        }

        else if( size<=(pre_masked&BLOCK_SIZE_MASK))
            return free_ptr;


            //now deal with allocation process and make sure everything in the programs syncs up to th


        }
        return 0;//if nothing is found then return 0 to  move on to the next list.
    }









