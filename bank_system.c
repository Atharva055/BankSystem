
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>

#define MAX_ACCOUNTS 100
#define MAX_TRANSACTIONS 100
#define FILENAME "bank_data.dat"
#define PIN_LENGTH 4
#define PASSWORD_LENGTH 8

typedef struct {
    char type[20];      // "Deposit", "Withdrawal", "Account Creation", etc.
    double amount;
    char date[20];
    char time[20];
} Transaction;

typedef struct {
    int accountNumber;
    char name[50];
    double balance;
    char pin[PIN_LENGTH + 1];
    char password[PASSWORD_LENGTH + 1];
    Transaction transactions[MAX_TRANSACTIONS];
    int transactionCount;
    int isActive;  // 1 for active, 0 for deleted
} Account;

Account accounts[MAX_ACCOUNTS];
int accountCount = 0;

// Function prototypes
void saveToFile();
void loadFromFile();
void clearInputBuffer();
int generateAccountNumber();
void getCurrentDateTime(char *dateStr, char *timeStr);
int validatePIN(const char *pin);
int validatePassword(const char *password);
void hidePassword(char *password);

void mainMenu();
void createAccount();
void deleteAccount();
void depositMoney();
void withdrawMoney();
void checkBalance();
void viewTransactionHistory();
int findAccount(int accountNumber);
void loginAccount();

int main() {
    loadFromFile();
    
    printf("\n==========================================\n");
    printf("     WELCOME TO BANK ACCOUNT SYSTEM\n");
    printf("==========================================\n");
    
    mainMenu();
    
    saveToFile();
    printf("\nThank you for using our banking system!\n");
    return 0;
}

void saveToFile() {
    FILE *file = fopen(FILENAME, "wb");
    if (file == NULL) {
        printf("Error saving data!\n");
        return;
    }
    
    fwrite(&accountCount, sizeof(int), 1, file);
    fwrite(accounts, sizeof(Account), accountCount, file);
    fclose(file);
}

void loadFromFile() {
    FILE *file = fopen(FILENAME, "rb");
    if (file == NULL) {
        printf("No existing data found. Starting fresh.\n");
        return;
    }
    
    fread(&accountCount, sizeof(int), 1, file);
    fread(accounts, sizeof(Account), accountCount, file);
    fclose(file);
}

void clearInputBuffer() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

int generateAccountNumber() {
    srand(time(NULL));
    return 100000 + rand() % 900000;  // 6-digit account number
}

void getCurrentDateTime(char *dateStr, char *timeStr) {
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    sprintf(dateStr, "%02d-%02d-%04d", tm.tm_mday, tm.tm_mon + 1, tm.tm_year + 1900);
    sprintf(timeStr, "%02d:%02d:%02d", tm.tm_hour, tm.tm_min, tm.tm_sec);
}

int validatePIN(const char *pin) {
    if (strlen(pin) != PIN_LENGTH) {
        return 0;
    }
    
    for (int i = 0; i < PIN_LENGTH; i++) {
        if (!isdigit(pin[i])) {
            return 0;
        }
    }
    return 1;
}

int validatePassword(const char *password) {
    int len = strlen(password);
    if (len < 8 || len > PASSWORD_LENGTH) {
        return 0;
    }
    
    int hasUpper = 0, hasLower = 0, hasDigit = 0;
    for (int i = 0; i < len; i++) {
        if (isupper(password[i])) hasUpper = 1;
        else if (islower(password[i])) hasLower = 1;
        else if (isdigit(password[i])) hasDigit = 1;
    }
    
    return (hasUpper && hasLower && hasDigit);
}

void hidePassword(char *password) {
    int i = 0;
    char ch;
    
    while (1) {
        ch = getchar();
        
        if (ch == '\n' || ch == '\r') {
            password[i] = '\0';
            break;
        } else if (ch == 8 || ch == 127) {  // Backspace
            if (i > 0) {
                i--;
                printf("\b \b");
            }
        } else {
            if (i < PASSWORD_LENGTH) {
                password[i] = ch;
                i++;
                printf("*");
            }
        }
    }
}

void mainMenu() {
    int choice;
    
    while (1) {
        printf("\n========== MAIN MENU ==========\n");
        printf("1. Create New Account\n");
        printf("2. Login to Account\n");
        printf("3. Delete Account\n");
        printf("4. Deposit Money\n");
        printf("5. Withdraw Money\n");
        printf("6. Check Balance\n");
        printf("7. View Transaction History\n");
        printf("8. Exit\n");
        printf("===============================\n");
        printf("Enter your choice: ");
        
        if (scanf("%d", &choice) != 1) {
            printf("Invalid input! Please enter a number.\n");
            clearInputBuffer();
            continue;
        }
        
        clearInputBuffer();
        
        switch (choice) {
            case 1:
                createAccount();
                break;
            case 2:
                loginAccount();
                break;
            case 3:
                deleteAccount();
                break;
            case 4:
                depositMoney();
                break;
            case 5:
                withdrawMoney();
                break;
            case 6:
                checkBalance();
                break;
            case 7:
                viewTransactionHistory();
                break;
            case 8:
                return;
            default:
                printf("Invalid choice! Please try again.\n");
        }
    }
}

void createAccount() {
    if (accountCount >= MAX_ACCOUNTS) {
        printf("Maximum account limit reached!\n");
        return;
    }
    
    Account newAccount;
    newAccount.accountNumber = generateAccountNumber();
    newAccount.balance = 0.0;
    newAccount.transactionCount = 0;
    newAccount.isActive = 1;
    
    printf("\n======== CREATE ACCOUNT ========\n");
    printf("Account Number: %d\n", newAccount.accountNumber);
    
    printf("Enter your name: ");
    fgets(newAccount.name, sizeof(newAccount.name), stdin);
    newAccount.name[strcspn(newAccount.name, "\n")] = 0;
    
    // Get and validate PIN
    while (1) {
        printf("Enter 4-digit PIN: ");
        fgets(newAccount.pin, sizeof(newAccount.pin), stdin);
        newAccount.pin[strcspn(newAccount.pin, "\n")] = 0;
        
        if (validatePIN(newAccount.pin)) {
            break;
        }
        printf("Invalid PIN! Must be exactly 4 digits.\n");
    }
    
    // Get and validate password
    while (1) {
        printf("Enter password (8 chars, mix of upper/lower/digits): ");
        hidePassword(newAccount.password);
        
        if (validatePassword(newAccount.password)) {
            break;
        }
        printf("\nInvalid password! Must be 8 characters with uppercase, lowercase, and digits.\n");
    }
    
    // Add creation transaction
    char date[20], currentTime[20];
    getCurrentDateTime(date, currentTime);
    
    Transaction creationTrans;
    strcpy(creationTrans.type, "Account Creation");
    creationTrans.amount = 0.0;
    strcpy(creationTrans.date, date);
    strcpy(creationTrans.time, currentTime);
    
    newAccount.transactions[newAccount.transactionCount++] = creationTrans;
    
    accounts[accountCount++] = newAccount;
    saveToFile();
    
    printf("\nAccount created successfully!\n");
    printf("Your Account Number: %d\n", newAccount.accountNumber);
    printf("Keep your PIN and password secure!\n");
}

int findAccount(int accountNumber) {
    for (int i = 0; i < accountCount; i++) {
        if (accounts[i].accountNumber == accountNumber && accounts[i].isActive) {
            return i;
        }
    }
    return -1;
}

void loginAccount() {
    int accountNumber;
    char pin[PIN_LENGTH + 1];
    char password[PASSWORD_LENGTH + 1];
    
    printf("\n========== LOGIN ==========\n");
    printf("Enter Account Number: ");
    scanf("%d", &accountNumber);
    clearInputBuffer();
    
    int index = findAccount(accountNumber);
    if (index == -1) {
        printf("Account not found or inactive!\n");
        return;
    }
    
    printf("Enter PIN: ");
    hidePassword(pin);
    
    printf("\nEnter Password: ");
    hidePassword(password);
    
    if (strcmp(accounts[index].pin, pin) == 0 && 
        strcmp(accounts[index].password, password) == 0) {
        printf("\nLogin successful!\n");
        
        // Show account menu after login
        int choice;
        do {
            printf("\n======== ACCOUNT MENU ========\n");
            printf("1. Deposit Money\n");
            printf("2. Withdraw Money\n");
            printf("3. Check Balance\n");
            printf("4. View Transactions\n");
            printf("5. Logout\n");
            printf("==============================\n");
            printf("Enter choice: ");
            scanf("%d", &choice);
            clearInputBuffer();
            
            switch (choice) {
                case 1:
                    depositMoney();
                    break;
                case 2:
                    withdrawMoney();
                    break;
                case 3:
                    checkBalance();
                    break;
                case 4:
                    viewTransactionHistory();
                    break;
                case 5:
                    printf("Logged out successfully!\n");
                    break;
                default:
                    printf("Invalid choice!\n");
            }
        } while (choice != 5);
    } else {
        printf("\nInvalid PIN or Password!\n");
    }
}

void depositMoney() {
    int accountNumber;
    double amount;
    
    printf("\n========== DEPOSIT ==========\n");
    printf("Enter Account Number: ");
    scanf("%d", &accountNumber);
    
    int index = findAccount(accountNumber);
    if (index == -1) {
        printf("Account not found!\n");
        clearInputBuffer();
        return;
    }
    
    // Verify PIN
    char pin[PIN_LENGTH + 1];
    printf("Enter PIN: ");
    scanf("%s", pin);
    
    if (strcmp(accounts[index].pin, pin) != 0) {
        printf("Invalid PIN!\n");
        clearInputBuffer();
        return;
    }
    
    printf("Enter amount to deposit: $");
    scanf("%lf", &amount);
    clearInputBuffer();
    
    if (amount <= 0) {
        printf("Invalid amount! Must be positive.\n");
        return;
    }
    
    accounts[index].balance += amount;
    
    // Add transaction record
    char date[20], currentTime[20];
    getCurrentDateTime(date, currentTime);
    
    Transaction trans;
    strcpy(trans.type, "Deposit");
    trans.amount = amount;
    strcpy(trans.date, date);
    strcpy(trans.time, currentTime);
    
    if (accounts[index].transactionCount < MAX_TRANSACTIONS) {
        accounts[index].transactions[accounts[index].transactionCount++] = trans;
    }
    
    saveToFile();
    printf("Deposit successful! New balance: $%.2f\n", accounts[index].balance);
}

void withdrawMoney() {
    int accountNumber;
    double amount;
    
    printf("\n========== WITHDRAW ==========\n");
    printf("Enter Account Number: ");
    scanf("%d", &accountNumber);
    
    int index = findAccount(accountNumber);
    if (index == -1) {
        printf("Account not found!\n");
        clearInputBuffer();
        return;
    }
    
    // Verify PIN
    char pin[PIN_LENGTH + 1];
    printf("Enter PIN: ");
    scanf("%s", pin);
    
    if (strcmp(accounts[index].pin, pin) != 0) {
        printf("Invalid PIN!\n");
        clearInputBuffer();
        return;
    }
    
    printf("Enter amount to withdraw: $");
    scanf("%lf", &amount);
    clearInputBuffer();
    
    if (amount <= 0) {
        printf("Invalid amount! Must be positive.\n");
        return;
    }
    
    if (amount > accounts[index].balance) {
        printf("Insufficient balance! Available: $%.2f\n", accounts[index].balance);
        return;
    }
    
    accounts[index].balance -= amount;
    
    // Add transaction record
    char date[20], currentTime[20];
    getCurrentDateTime(date, currentTime);
    
    Transaction trans;
    strcpy(trans.type, "Withdrawal");
    trans.amount = amount;
    strcpy(trans.date, date);
    strcpy(trans.time, currentTime);
    
    if (accounts[index].transactionCount < MAX_TRANSACTIONS) {
        accounts[index].transactions[accounts[index].transactionCount++] = trans;
    }
    
    saveToFile();
    printf("Withdrawal successful! New balance: $%.2f\n", accounts[index].balance);
}

void checkBalance() {
    int accountNumber;
    
    printf("\n======== CHECK BALANCE ========\n");
    printf("Enter Account Number: ");
    scanf("%d", &accountNumber);
    
    int index = findAccount(accountNumber);
    if (index == -1) {
        printf("Account not found!\n");
        clearInputBuffer();
        return;
    }
    
    // Verify PIN
    char pin[PIN_LENGTH + 1];
    printf("Enter PIN: ");
    scanf("%s", pin);
    
    if (strcmp(accounts[index].pin, pin) != 0) {
        printf("Invalid PIN!\n");
        clearInputBuffer();
        return;
    }
    
    printf("\nAccount Holder: %s\n", accounts[index].name);
    printf("Account Number: %d\n", accounts[index].accountNumber);
    printf("Current Balance: $%.2f\n", accounts[index].balance);
}

void viewTransactionHistory() {
    int accountNumber;
    
    printf("\n===== TRANSACTION HISTORY =====\n");
    printf("Enter Account Number: ");
    scanf("%d", &accountNumber);
    
    int index = findAccount(accountNumber);
    if (index == -1) {
        printf("Account not found!\n");
        clearInputBuffer();
        return;
    }
    
    // Verify PIN
    char pin[PIN_LENGTH + 1];
    printf("Enter PIN: ");
    scanf("%s", pin);
    
    if (strcmp(accounts[index].pin, pin) != 0) {
        printf("Invalid PIN!\n");
        clearInputBuffer();
        return;
    }
    
    printf("\nAccount Holder: %s\n", accounts[index].name);
    printf("Account Number: %d\n", accounts[index].accountNumber);
    printf("\nTransaction History:\n");
    printf("------------------------------------------------------------\n");
    printf("Type\t\tAmount\t\tDate\t\tTime\n");
    printf("------------------------------------------------------------\n");
    
    for (int i = 0; i < accounts[index].transactionCount; i++) {
        Transaction trans = accounts[index].transactions[i];
        printf("%-15s $%-10.2f %-12s %-12s\n", 
               trans.type, trans.amount, trans.date, trans.time);
    }
    
    if (accounts[index].transactionCount == 0) {
        printf("No transactions found.\n");
    }
    printf("------------------------------------------------------------\n");
}

void deleteAccount() {
    int accountNumber;
    char pin[PIN_LENGTH + 1];
    char password[PASSWORD_LENGTH + 1];
    
    printf("\n======== DELETE ACCOUNT ========\n");
    printf("Enter Account Number: ");
    scanf("%d", &accountNumber);
    clearInputBuffer();
    
    int index = findAccount(accountNumber);
    if (index == -1) {
        printf("Account not found!\n");
        return;
    }
    
    // Double verification for security
    printf("Enter PIN: ");
    hidePassword(pin);
    
    printf("\nEnter Password: ");
    hidePassword(password);
    
    if (strcmp(accounts[index].pin, pin) == 0 && 
        strcmp(accounts[index].password, password) == 0) {
        
        // Confirm deletion
        char confirm;
        printf("\nWARNING: This will permanently delete your account!\n");
        printf("Are you sure? (y/n): ");
        scanf(" %c", &confirm);
        clearInputBuffer();
        
        if (confirm == 'y' || confirm == 'Y') {
            accounts[index].isActive = 0;  // Mark as deleted
            
            // Add deletion transaction
            char date[20], currentTime[20];
            getCurrentDateTime(date, currentTime);
            
            Transaction trans;
            strcpy(trans.type, "Account Deletion");
            trans.amount = accounts[index].balance;
            strcpy(trans.date, date);
            strcpy(trans.time, currentTime);
            
            if (accounts[index].transactionCount < MAX_TRANSACTIONS) {
                accounts[index].transactions[accounts[index].transactionCount++] = trans;
            }
            
            saveToFile();
            printf("Account deleted successfully!\n");
        } else {
            printf("Account deletion cancelled.\n");
        }
    } else {
        printf("\nInvalid PIN or Password! Deletion failed.\n");
    }
}
