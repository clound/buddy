#include <iostream>
#include <map>
#include <tuple>
#include <string>
#include <cmath>
using namespace std;

#define MAX_ORDER 11

map<string,tuple<int,int,struct page*>> process_info;

struct page{
    struct page *lru;
};

struct list_head{
    struct page *next;
};

struct free_area{
    struct list_head free_list;
    unsigned long nr_free;
};

struct zone{
    struct free_area free_area[MAX_ORDER];
};


bool list_empty(struct free_area *area){
    if(area->nr_free==0)
        return true;
    else
        return false;
}

struct page* list_entry(struct free_area *area){
    return area->free_list.next;
}

void list_del(struct free_area *area){
    struct page *page = area->free_list.next;
    area->free_list.next=page->lru;
    area->nr_free--;
}

void list_add(struct page *page,struct free_area *area){
    page->lru=area->free_list.next;
    area->free_list.next=page;
    area->nr_free++;
}
unsigned int power2(unsigned int current_ord){
	unsigned int temp=1;
	for(unsigned int i=0;i<current_ord;i++)
		temp*=2;
	return temp;
}
struct page* alloc_memory(unsigned int order,struct zone *zone){
    struct page *new_page;
    struct free_area *area=NULL;
    unsigned int current_order;
    for (current_order=order; current_order<MAX_ORDER; ++current_order) {//一直往下寻找适合的区域
        area = zone->free_area + current_order;//zone数组定义后，取得当前的order号
        if (!list_empty(area)){//判断此块是否为空
            new_page=list_entry(area);//取出此页
            list_del(area);//对此块进行处理
            struct page *buddy;
            if(current_order>0){
                current_order--;//将当前的号减1，因为当前块是适合的区域
                while(current_order>=order){//判断出是否适合到达要求的order号
                    area = zone->free_area + current_order;
                    buddy= new_page+power2(current_order);//对其进行一半分割处理，取出和向前链接
                    list_add(buddy,area);//在前面的减1的order号尾部链接
                    if(current_order==0)
                        break;
                    else
                        current_order--;//循环内继续减1
                }
            }
            return new_page;
        }
    }
    return NULL;
}

bool isBuddy(unsigned int page1,unsigned int order1,unsigned int page2,unsigned int order2)
{
	int num=power2((order1+1)*page1);
	int num1=power2((order2+1)*page2)+power2(page2);
	int elm=power2(order1+1);
	if(order1==order2){
		if(page1 < page2){	 
			if(page1%elm==0)
				{
					if(page1+power2(order2)==page2)
						return true;
				}
			}
		else{
			if(page2%elm==0)
			{
				if(page1-page2==power2(order2))
					return true;	
				}
		}
    	}
	return false;
}
void release_memory(unsigned int order,struct zone *zone,struct page *new_page,struct page *addr){
    struct page *temp=NULL;
    struct free_area *area=NULL;
    struct page *page=NULL;
    unsigned int page_count,page_count2;
    while(order<MAX_ORDER){
        area=zone->free_area+order;
        page=area->free_list.next;
        page_count=(int)(new_page-addr);
        bool buddy_flag=false;
        while(page!=NULL){
			page_count2=(int)(page-addr);
			buddy_flag=(bool)isBuddy(page_count,order,page_count2,order);
			if(buddy_flag)
			{
				 if(order==MAX_ORDER-1)
                   			 break;
				if(temp==NULL)
                    area->free_list.next=page->lru;
                else
                    temp->lru=page->lru;
				area->nr_free--;
				if(page_count>page_count2){
                    new_page=page;
               
                }
                break;
			}
            temp=page;
            page=page->lru;
        }
        if(buddy_flag)
            order++;
        else
            break;
    }
    list_add(new_page, area);
    return;
}

void check_memory(struct zone *zone,struct page *addr){
    struct free_area *area;

    for(int i=0;i<11;i++){
        area=zone->free_area+i;
        cout<<"order:"<<i;
        cout<<"   nr_free:"<<area->nr_free<<endl;
        struct page* page=area->free_list.next;
        while(page!=NULL){
            cout<<" "<<page-addr<<"~"<<page-addr+power2(i)-1<<endl;
            page=page->lru;
        }
    }
}

void check_process(struct page *addr){
    map<string,tuple<int,int,struct page*>>::iterator it;
    for(it=process_info.begin();it!=process_info.end();++it){
        cout<<"ID: "<<it->first<<endl;
        cout<<"size:"<<get<0>(it->second)<<"KB"<<endl;
        unsigned int order=get<1>(it->second);
        cout<<"order:"<<order<<endl;
        cout<<"addr:"<<get<2>(it->second)-addr<<"~"<<get<2>(it->second)-addr+power2(order)-1<<endl;
    }
}


unsigned int order_init(unsigned int size){
	int order=1;
	while(4*power2(order-1)<size)
		{
			order++;
		}
	if(order<=10)
	  return order-1;

}

int main(int argc, const char * argv[]) {
    
    struct zone *zone=(struct zone*)malloc(sizeof(struct zone));
    struct free_area *area=NULL;
    struct page *addr=(struct page*)malloc(62*1024*sizeof(struct page));
    
    for(int i=0;i<10;i++){
        area=zone->free_area+i;
        area->nr_free=0;
        area->free_list.next=NULL;
    }
    area=zone->free_area+10;
    area->free_list.next=addr;
    area->nr_free=64;
    struct page *init_page=addr;
    for(int i=0;i<63;i++){
        init_page->lru=init_page+1024;
        init_page=init_page->lru;
    }
    cout<<"Memory Init Completed!"<<endl;
   
    /*
     *   check_memory:
     *       0
     *   alloc_memory:
     *       1 size process_id
     *   release_memory:
     *       2 process_id
     *   check_process:
     *       3
     *   quit:
     *       default
     *   size 单位为kb
     */
    
    int option;
    unsigned int size;
    string process_id;
    
    while(true){
        
		cout<<"-For check_memory:     0"<<endl;
		cout<<"-For alloc_memory:     1 size(KB) process_name"<<endl;
		cout<<"-For release_memory:   2 process_name"<<endl;
		cout<<"-For check_process:    3"<<endl;
		cout<<"-For quit:             default"<<endl;
		cout<<"Input your command:    ";
		
        cin>>option;

		cout << endl;
        switch (option) {
            case 0:
                check_memory(zone, addr);
                break;
            case 1:{
                cin>>size>>process_id;
                unsigned int order=order_init(size);
                if(process_info.find(process_id)==process_info.end()){
                    struct page *new_page=alloc_memory(order, zone);
                    if(new_page==NULL){
                        cout<<"No enough memory,please release memory first!"<<endl;
                    }
                    else{
                        process_info[process_id]=make_tuple(size,order,new_page);
                        cout<<"Alloc Memory Completed!"<<endl;
                        cout<<"ID: "<<process_id<<endl;
                        cout<<"size:"<<size<<"KB"<<endl;
                        cout<<"order:"<<order<<endl;
                        cout<<"addr:"<<new_page-addr<<"~"<<new_page-addr+power2(order)-1<<endl;
                    }
                }
                else
                    cout<<"Process "<<process_id<<" is existed!"<<endl;
                break;
            }
            case 2:{
                cin>>process_id;
                if(process_info.find(process_id)!=process_info.end()){
                    unsigned int order=get<1>(process_info[process_id]);
                    struct page *new_page=get<2>(process_info[process_id]);
                    release_memory(order, zone, new_page, addr);
                    process_info.erase(process_id);
                    cout<<"Process "<<process_id<<" has been released!"<<endl;
                }
                else
                    cout<<"Process "<<process_id<<" is not existed!"<<endl;
                break;
            }
            case 3:
                check_process(addr);
                break;
            default:
                return 0;
        }
		cout << endl;
    }
    return 0;
}
