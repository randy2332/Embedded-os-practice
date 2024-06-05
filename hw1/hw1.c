#include <stdio.h>
#include <stdlib.h>
#include <string.h>
# include <unistd.h>
# include <fcntl.h>
// 定義餐廳結構
struct Restaurant {
    char name[50];
    int distance;
    char menu_item1[50];
    int price1;
    char menu_item2[50];
    int price2;
};

// 初始化餐廳資訊
struct Restaurant restaurants[3] = {
    {"Dessert shop", 3, "cookie", 60, "cake", 80},
    {"Beverage shop", 5, "tea", 40, "boba", 70},
    {"Diner", 8, "fried rice", 120, "egg-drop soup", 50}
};

int main() {
    int choice;
    int restaurant_choice;
    int item_choice;
    int quantity;
    int total_amount = 0;
    int continue_order = 1;

    //open led device
    int fd_led = open("/dev/etx_device",O_WRONLY);
    if(fd_led<0){
        printf("%s","Can not open the file(led device)");
    }
    
    // seven seg 
    int fd_seg = open("/dev/mydev",O_WRONLY);
    if(fd_seg<0){
        printf("%s","Can not open the file(seven seg device)");
    }

    while (1) {
        // 主選單
        printf("\nMain Menu\n");
        printf("1. Shop List\n");
        printf("2. Order\n");
        printf("Enter your choice: ");
        scanf("%d", &choice);

        switch (choice) {
            case 1:
                // 顯示餐廳與距離
                printf("\nShop List\n");
                printf("Dessert shop: 3km\n");
                printf("Beverage shop: 5km\n");
                printf("Diner: 8km\n");
                printf("press any key to return to the main menu");
                getchar();
                getchar();
                //getchar();
                break;
            case 2:
                // 訂購
		int shopping_cart = 0; // 是否有點餐 1: 有點餐; 0: 沒有點餐
                int confirm_order = 0; // 是否確認訂單 1: 確認訂單; 0: 取消訂單
                while (continue_order) {
                    
                    if (shopping_cart==0 && confirm_order==0){
		        printf("\nOrder\n");
                        printf("Please choose from 1~3:\n");
                        printf("1. Dessert shop\n");
                        printf("2. Beverage Shop\n");
                        printf("3. Diner\n");
                        printf("Enter your choice: ");
                        scanf("%d", &restaurant_choice);
                          // 選擇餐廳
		        if (restaurant_choice < 1 || restaurant_choice > 3) {
		            printf("Invalid choice.\n");
		            break;
                        }
                        confirm_order =1;
                    }

                  
		
                    printf("Please choose from 1~4:\n");
                    printf("1. %s: $%d\n", restaurants[restaurant_choice-1].menu_item1,restaurants[restaurant_choice-1].price1);
                    printf("2. %s: $%d\n", restaurants[restaurant_choice-1].menu_item2,restaurants[restaurant_choice-1].price2);
                    printf("3. confirm\n");
                    printf("4. cancel\n");
                    printf("Enter your choice: ");
                    scanf("%d", &item_choice);

                    // 選擇餐點
                    if (item_choice < 1 || item_choice > 4) {
                        printf("Invalid choice.\n");
                        break;
                    }

                    if (item_choice == 4) {
                        printf("Order canceled.\n");
                        total_amount = 0;
                        break;
                    }
                    
		    if (item_choice==1 || item_choice==2)
		    {
		        printf("How many? ");
                    	scanf("%d", &quantity);
		    }


                    // 計算訂單總金額
                    if (item_choice == 1) {
                        total_amount += restaurants[restaurant_choice-1].price1 * quantity;
                        shopping_cart==1;
                    } else if (item_choice == 2) {
                        total_amount += restaurants[restaurant_choice-1].price2 * quantity;
                        shopping_cart==1;
                    } else if (item_choice == 3 && total_amount!=0) {
                        printf("Total amount: $%d\n", total_amount);
                        // seven seg
                        
                        write(fd_seg,&total_amount,sizeof(int));
                        printf("Starting delivery...\n"); 
                        printf("Please wait for a few minutes...\n");
                        //led light
                        int distance = restaurants[restaurant_choice-1].distance;//distnace of restaurants
                       
                        write(fd_led,&distance,1);
                        
                        printf("Meal delivered! Please pick up your meal.\n");
                        total_amount = 0; // 訂單完成後重置總金額
                        continue_order = 0;
                    }else if (item_choice == 3 && total_amount==0) {
                    	printf("You haven't ordered anything, please place an order again.\n");
                    	shopping_cart=0;
                    	confirm_order=0;
                    }
                    
                }
                continue_order = 1; // 重置點餐狀態
                break;
            default:
                printf("Invalid choice.\n");
        }
    }

    return 0;
}

