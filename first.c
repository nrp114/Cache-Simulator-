#include<stdlib.h>
#include <stdio.h>
#include<string.h>
#include<stdbool.h>
#include<math.h>


	int writes=0;
	int reads=0;
	int hits=0;
	int miss=0;

	int writes_pre=0;
	int reads_pre =0;
	int hits_pre =0;
	int miss_pre =0;

struct node{
    unsigned long long int address;
    int valid;
    struct node* next;
};

bool isproper(int num);
void cash_write(struct node** cache, unsigned long long int address, char command, int block_size, int number_of_set,int cash_policy,int assoc);
void cash_read(struct node** cache, unsigned long long int address, char command, int block_size, int number_of_set, int cash_policy, int assoc);
void cash_read_p(struct node** cache, unsigned long long int address, char command, int block_size, int number_of_set, int cash_policy, int prefetch_size, int assoc);
void cash_write_pre(struct node** cache, unsigned long long int address, char command, int block_size, int number_of_set, int cash_policy, int prefetch_size, int assoc);
void cash_read_pre(struct node** cache, unsigned long long int address, char command, int block_size, int number_of_set, int cash_policy, int assoc);
void print_cache(struct node** cache, int number_of_sets);
void free_cache(struct node** cache, int number_of_sets, int assoc);

struct node** create_cash(int number_of_set, int assoc){
    
    struct node** cache = malloc(sizeof(struct node*)*number_of_set);
   
    for(int i=0; i<number_of_set; i++){
        int count_block = 0;
        struct node* ptr = cache[i];
        while(count_block != assoc){
            struct node* block_node = malloc(sizeof(struct node));
            block_node->address = 0;
	    block_node->valid = 0;
            block_node->next = NULL;
       
	if(ptr == NULL){
	cache[i] = block_node;
	ptr=block_node;
	}
	else{
		ptr->next = block_node;
		ptr = ptr->next;
	}   
            count_block++;
            
        }
        
    }
    return cache;
}

bool isproper(int num){
    
    while(num!=1){
        if(num%2!=0){
            return false;
        }   
        num=num/2;
    }
   return true;
}


int main(int argc, char** argv)
{
    if(argc != 7 || isproper(atoi(argv[1]))==false || isproper(atoi(argv[2]))==false){
        printf("error\n");
        exit(0);
    }
    int cash_size = atoi(argv[1]);
    int block_size = atoi(argv[2]);
    
    int cash_policy = 0;
    
    if(argv[3][0]=='f'){
        cash_policy = 1;
    }
    else if(argv[3][0]=='l'){
        cash_policy = 2;
    }
    else{
        printf("error\n");
        exit(0);
    }
    
    int assoc = 0;
    int number_of_set = 0;
    
    if(argv[4][0]=='d'){
        assoc = 1;
        number_of_set = cash_size/block_size;
    }
    else if(argv[4][0]=='a'){
        
        if(argv[4][5] == '\0'){
            number_of_set = 1;
            assoc = cash_size/block_size;
        }
        else if(argv[4][5] == ':'){
            sscanf(argv[4],"assoc:%d", &assoc);
		if(isproper(assoc) == false){
			printf("error\n");
			exit(0);
		}
            number_of_set = cash_size/(block_size*assoc);
        }
        
    }
    else{
        printf("error\n");
        exit(0);
    }
    
    int prefetch_size =atoi(argv[5]);
    
    FILE* fp = fopen(argv[6],"r");
    	if(fp == NULL){ 
	       printf("error\n");
              exit(0);
	    }	  
	
	struct node** cache = create_cash(number_of_set,assoc);  
	//print_cache(cache,number_of_set);
	char command;
	unsigned long long int address;
	

	while(fscanf(fp, "%c %llx\n", &command, &address)>0){
		if(command =='#'){
			break;
		}
		if(command == 'W'){
		
		cash_write(cache,address,command, block_size,number_of_set,cash_policy,assoc);
				
		}
		if(command == 'R'){
		//printf("%d\n", reads);
		cash_read(cache,address,command, block_size, number_of_set, cash_policy,assoc);
		}
	}

	fclose(fp);


	  FILE* fpp = fopen(argv[6],"r");
    	if(fpp == NULL){ 
	       printf("error\n");
              exit(0);
	    }	  
	
	struct node** cache_pre = create_cash(number_of_set,assoc);  

	char command_pre;
	unsigned long long int address_pre;
	while(fscanf(fpp, "%c %llx\n", &command_pre, &address_pre)>0){
		if(command_pre =='#'){
			break;
		}
		if(command_pre == 'W'){
		
		cash_write_pre(cache_pre,address_pre,command_pre, block_size,number_of_set,cash_policy,prefetch_size,assoc);
				
		}
		if(command_pre == 'R'){
		
		cash_read_p(cache_pre,address_pre,command_pre, block_size, number_of_set, cash_policy,prefetch_size,assoc);
		}
	}
	fclose(fpp);
	printf("no-prefetch\nMemory reads: %d\nMemory writes: %d\nCache hits: %d\nCache misses: %d\n",reads,writes, hits, miss);
	printf("with-prefetch\nMemory reads: %d\nMemory writes: %d\nCache hits: %d\nCache misses: %d\n",reads_pre,writes_pre, hits_pre, miss_pre);


	free_cache(cache, number_of_set,assoc);
	free_cache(cache_pre,number_of_set, assoc);

}

void cash_write_pre(struct node** cache, unsigned long long int address, char command, int block_size, int number_of_set, int cash_policy, int prefetch_size, int assoc){

	unsigned long long int set_b = 0;
	unsigned long long int temp = address;
	int b_size = log(block_size)/log(2);
	int s_size = log(number_of_set)/log(2);

	temp = temp >> b_size;
	if(number_of_set == 1){
		set_b=0;
	}
	else{
		set_b = temp & ((1<< s_size)-1);
	}
	
	temp = temp >> s_size;

	struct node* n = cache[set_b];
	struct node* ptr = n;
	struct node* prev = ptr;
	//struct node* pre_prev = prev;
	while(ptr != NULL){
		if( ptr->valid == 0){
			miss_pre++;
			reads_pre++;
			writes_pre++;
			ptr->address = temp;
			ptr->valid = 1;
			
			while(prefetch_size != 0){
			address = address + block_size;
			cash_read_pre(cache, address, command, block_size, number_of_set, cash_policy,assoc);
			prefetch_size--;
			}

			return;
		}
		else if(ptr->address == temp){
			hits_pre++;
			writes_pre++;

			if(cash_policy == 2){
        
       				 struct node* rear = ptr;
           
				while(rear->next != NULL && rear->next->valid == 1){
							/*if(rear->next->valid == 0){
								break;
							}*/
	               				 rear = rear->next;
							}

						if(rear == ptr){
							return;
						}
         					if(ptr == prev){
						cache[set_b]= ptr->next;
						ptr->next = rear->next;
						rear->next=ptr;
						}
        
				else if(rear != ptr){
				    prev->next = ptr->next;
					ptr->next = rear->next;
				    rear->next = ptr;
				}

			}

			return;
			

		}
		if(assoc == 1){
		break;
	}
		//pre_prev = prev;
		prev = ptr;
		ptr = ptr->next;
	}

struct node* new_node = malloc(sizeof(struct node));

	if(assoc == 1){
		new_node->address = temp;	
		new_node->valid = 1;	
		cache[set_b] = new_node;
		new_node->next = NULL;
		free(prev);
		miss_pre++;
		reads_pre++;
		writes_pre++;
	}
	else{	
	prev->next = new_node;
	
	
	new_node->address = temp;
	new_node->valid = 1;
	new_node->next = NULL;
	cache[set_b] = n->next;
	free(n);
			miss_pre++;
			reads_pre++;
			writes_pre++;
	}
		while(prefetch_size != 0){
			address = address + block_size;
			cash_read_pre(cache, address, command, block_size, number_of_set, cash_policy, assoc);
			prefetch_size--;
			}
}



void cash_read_p(struct node** cache, unsigned long long int address, char command, int block_size, int number_of_set, int cash_policy, int prefetch_size, int assoc){
		
	unsigned long long int set_b = 0;
	unsigned long long int temp = address;
	int b_size = log(block_size)/log(2);
	int s_size = log(number_of_set)/log(2);

	temp = temp >> b_size;
	if(number_of_set == 1){
		set_b=0;
	}
	else{
		set_b = temp & ((1<< s_size)-1);
	}
	
	temp = temp >> s_size;

	struct node* n = cache[set_b];
	struct node* ptr = n;
	struct node* prev = ptr;
	//struct node* pre_prev = prev;

	while(ptr != NULL){
		if( ptr->valid == 0){
			miss_pre++;
			reads_pre++;
			ptr->address = temp;
			ptr->valid=1;

			while(prefetch_size != 0){
			address = address + block_size;
			cash_read_pre(cache, address, command, block_size, number_of_set, cash_policy,assoc);
			prefetch_size--;
			}
			return;
		}
		else if(ptr->address == temp){
			hits_pre++;

				if(cash_policy == 2){
        
       				 struct node* rear = ptr;
           
				while(rear->next != NULL && rear->next->valid == 1){
							/*if(rear->next->valid == 0){
								break;
							}*/
	               				 rear = rear->next;
							}

						if(rear == ptr){
							return;
						}
         					if(ptr == prev){
						cache[set_b]= ptr->next;
						ptr->next = rear->next;
						rear->next=ptr;
						}
        
				else if(rear != ptr){
				    prev->next = ptr->next;
					ptr->next = rear->next;
				    rear->next = ptr;
				}

			}

			return;
		}
		if(assoc == 1){
		break;
	}
		//pre_prev = prev;
		prev = ptr;
		ptr = ptr->next;
	}

struct node* new_node = malloc(sizeof(struct node));
	
	if(assoc == 1){
		new_node->address = temp;
		new_node->valid = 1;		
		cache[set_b] = new_node;
		new_node->next = NULL;
		free(prev);
		miss_pre++;
		reads_pre++;
	}
	else{
		prev->next = new_node;
	
	
	new_node->address = temp;
	new_node->valid = 1;
	new_node->next = NULL;
	cache[set_b] = n->next;
	free(n);
			miss_pre++;
			reads_pre++;
	}
		while(prefetch_size != 0){
			address = address + block_size;
			cash_read_pre(cache, address, command, block_size, number_of_set, cash_policy,assoc);
			prefetch_size--;
			}
}





void cash_read_pre(struct node** cache, unsigned long long int address, char command, int block_size, int number_of_set, int cash_policy, int assoc){
		
	unsigned long long int set_b = 0;
	unsigned long long int temp = address;
	int b_size = log(block_size)/log(2);
	int s_size = log(number_of_set)/log(2);

	temp = temp >> b_size;
	if(number_of_set == 1){
		set_b=0;
	}
	else{
		set_b = temp & ((1<< s_size)-1);
	}
	
	temp = temp >> s_size;

	struct node* n = cache[set_b];
	struct node* ptr = n;
	struct node* prev = ptr;
	//struct node* pre_prev = prev;

	while(ptr != NULL){
		if( ptr->valid == 0){
			//miss++;
			reads_pre++;
			ptr->address = temp;
			ptr->valid = 1;
			return;
		}
		else if(ptr->address == temp){
			//hits++;
			return;
		}
		if(assoc == 1){
		break;
		}
		//pre_prev = prev;
		prev = ptr;
		ptr = ptr->next;
	}

struct node* new_node = malloc(sizeof(struct node));

	if(assoc == 1){
		new_node->address = temp;
		new_node->valid = 1;		
		cache[set_b] = new_node;
		new_node->next = NULL;
		free(prev);
		//miss++;
		reads_pre++;
		//writes++;
		return;
	}

	prev->next = new_node;
	
	
	new_node->address = temp;
	new_node->valid = 1;
	new_node->next = NULL;
	cache[set_b] = n->next;
	free(n);
			//miss++;
			reads_pre++;
}




















void cash_write(struct node** cache, unsigned long long int address, char command, int block_size, int number_of_set, int cash_policy, int assoc){
	
	unsigned long long int set_b = 0;
	unsigned long long int temp = address;
	int b_size = log(block_size)/log(2);
	int s_size = log(number_of_set)/log(2);

	temp = temp >> b_size;
	if(number_of_set == 1){
		set_b=0;
	}
	else{
		set_b = temp & ((1<< s_size)-1);
	}
	
	temp = temp >> s_size;

	struct node* n = cache[set_b];
	struct node* ptr = cache[set_b];
	struct node* prev = ptr;
	//struct node* pre_prev = prev;
	while(ptr != NULL){
		if( ptr->valid == 0){
			miss++;
			reads++;
			writes++;
			ptr->address = temp;
			ptr->valid=1;
			return;
		}
		else if(ptr->address == temp){
			hits++;
			writes++;

				if(cash_policy == 2){
        
       				 struct node* rear = ptr;
           
				while(rear->next != NULL && rear->next->valid == 1){
							/*if(rear->next->valid == 0){
								break;
							}*/
	               				 rear = rear->next;
							}
						if(rear == ptr){
							return;
						}
         					if(ptr == prev){
						cache[set_b]= ptr->next;
						ptr->next = rear->next;
						rear->next=ptr;
						}
        
				else if(rear != ptr){
				    prev->next = ptr->next;
					ptr->next = rear->next;
				    rear->next = ptr;
				}

			}

			return;
		}

		if(assoc == 1){
		break;
	}
		//pre_prev = prev;
		prev = ptr;
		ptr = ptr->next;
	}

struct node* new_node = malloc(sizeof(struct node));

	if(assoc == 1){
		new_node->address = temp;
		new_node->valid=1;		
		cache[set_b] = new_node;
		new_node->next = NULL;
		free(prev);
		miss++;
		reads++;
		writes++;
		return;
	}

	prev->next = new_node;
	
	
	new_node->address = temp;
	new_node->valid=1;
	new_node->next = NULL;
	cache[set_b] = n->next;
	free(n);
			miss++;
			reads++;
			writes++;
	
}

void cash_read(struct node** cache, unsigned long long int address, char command, int block_size, int number_of_set, int cash_policy, int assoc){
		
	unsigned long long int set_b = 0;
	unsigned long long int temp = address;
	int b_size = log(block_size)/log(2);
	int s_size = log(number_of_set)/log(2);

	temp = temp >> b_size;
	if(number_of_set == 1){
		set_b=0;
	}
	else{
		set_b = temp & ((1<< s_size)-1);
	}	

	temp = temp >> s_size;

	struct node* n = cache[set_b];
	struct node* ptr = cache[set_b];
	struct node* prev = ptr;
	//struct node* pre_prev = prev;

	while(ptr != NULL){
		if( ptr->valid == 0){
			miss++;
			reads++;
			ptr->address = temp;
			ptr->valid = 1;
			return;
		}
		else if(ptr->address == temp){
			hits++;

				if(cash_policy == 2){
        
       				 struct node* rear = ptr;
           
				while(rear->next != NULL && rear->next->valid == 1){
							/*if(rear->next->valid == 0){
								break;
							}*/
	               				 rear = rear->next;
							}

						if(rear == ptr){
							return;
						}
         					if(ptr == prev){
						cache[set_b]= ptr->next;
						ptr->next = rear->next;
						rear->next=ptr;
						}
        
				else if(rear != ptr){
				    prev->next = ptr->next;
					ptr->next = rear->next;
				    rear->next = ptr;
				}

			}

			return;
		}
		
		if(assoc == 1){
		break;
	}
		//pre_prev = prev;
		prev = ptr;
		ptr = ptr->next;
	}

struct node* new_node = malloc(sizeof(struct node));

	if(assoc == 1){
		new_node->address = temp;	
		new_node->valid=1;	
		cache[set_b] = new_node;
		new_node->next = NULL;
		free(prev);
		miss++;
		reads++;
		return;
	}

	prev->next = new_node;
	
	
	new_node->address = temp;
	new_node->valid = 1;
	new_node->next = NULL;
	cache[set_b] = n->next;
	free(n);
			miss++;
			reads++;
}

void print_cache(struct node** cache, int number_of_sets){
	int count=0;
	for(int i=0; i<number_of_sets; i++){
		struct node* ptr = cache[i];
		while(ptr!=NULL){
			printf("% d ", count);
			count++;
			ptr = ptr->next;
	}
		printf("\n");
	}

}

void free_cache(struct node** cache, int number_of_sets, int assoc){

	 for(int i=0; i<number_of_sets; i++){
      		  struct node* ptr = cache[i];
		struct node* prev = ptr;
			//  int count_block = 0;
        while(prev != NULL){
            	ptr = ptr->next;
       		free(prev);
		prev=ptr;
		}
}
	free(cache);
}





