#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <conio.h>

#define ID_LENGTH 6
#define NAME_LENGTH 25
#define FILENAME "components.dat"
#define PAGE_SIZE 20

typedef struct {
    char id[ID_LENGTH + 1];
    char name[NAME_LENGTH + 1];
    int stock;
    float price;
} Component;

void clearInput() { 
    while (getchar() != '\n'); 
}

void pause() { 
    printf("\nTekan Enter..."); 
    clearInput(); 
}

int validateId(const char *id) {
    if (strlen(id) != ID_LENGTH) {
        printf("ID harus tepat %d digit!\n", ID_LENGTH);
        return 0;
    }
    for (int i = 0; i < ID_LENGTH; i++) {
        if (!isdigit(id[i])) {
            printf("ID hanya boleh angka!\n");
            return 0;
        }
    }
    return 1;
}

int findComponent(const char *id, Component *comp, long *position) {
    FILE *file = fopen(FILENAME, "rb");
    if (!file) return 0;
    
    Component temp;
    long pos = 0;
    
    while (fread(&temp, sizeof(Component), 1, file)) {
        if (strcmp(temp.id, id) == 0) {
            if (comp) *comp = temp;
            if (position) *position = pos;
            fclose(file);
            return 1;
        }
        pos += sizeof(Component);
    }
    
    fclose(file);
    return 0;
}

void saveComponent(const Component *comp, long position) {
    FILE *file = fopen(FILENAME, position == -1 ? "ab" : "r+b");
    if (!file) {
        printf("Gagal membuka file!\n");
        return;
    }
    
    if (position != -1) fseek(file, position, SEEK_SET);
    fwrite(comp, sizeof(Component), 1, file);
    fclose(file);
}

int deleteComponent(const char *id) {
    FILE *file = fopen(FILENAME, "rb");
    FILE *temp = fopen("temp.dat", "wb");
    if (!file || !temp) {
        if (file) fclose(file);
        if (temp) fclose(temp);
        return 0;
    }
    
    Component comp;
    int found = 0;
    
    while (fread(&comp, sizeof(Component), 1, file)) {
        if (strcmp(comp.id, id) != 0) {
            fwrite(&comp, sizeof(Component), 1, temp);
        } else {
            found = 1;
        }
    }
    
    fclose(file);
    fclose(temp);
    
    if (found) {
        remove(FILENAME);
        rename("temp.dat", FILENAME);
    } else {
        remove("temp.dat");
    }
    
    return found;
}

int loadAllComponents(Component **components) {
    FILE *file = fopen(FILENAME, "rb");
    if (!file) return 0;
    
    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    if (size == 0) {
        fclose(file);
        return 0;
    }
    
    int count = size / sizeof(Component);
    *components = malloc(count * sizeof(Component));
    if (!*components) {
        fclose(file);
        return 0;
    }
    
    fseek(file, 0, SEEK_SET);
    fread(*components, sizeof(Component), count, file);
    fclose(file);
    
    return count;
}

void sortComponents(Component *components, int count) {
    for (int i = 0; i < count - 1; i++) {
        for (int j = i + 1; j < count; j++) {
            if (strcmp(components[i].id, components[j].id) > 0) {
                Component temp = components[i];
                components[i] = components[j];
                components[j] = temp;
            }
        }
    }
}

void sortByName(Component *components, int count) {
    for (int i = 0; i < count - 1; i++) {
        for (int j = i + 1; j < count; j++) {
            if (strcmp(components[i].name, components[j].name) > 0) {
                Component temp = components[i];
                components[i] = components[j];
                components[j] = temp;
            }
        }
    }
}


void addComponent() {
    Component comp;
    char tempId[100];

    while (1) {
        printf("\nID komponen (6 digit): ");
        scanf("%s", tempId);
        clearInput();

        if (strlen(tempId) < ID_LENGTH) {
            char padded[ID_LENGTH + 1];
            sprintf(padded, "%0*s", ID_LENGTH, tempId);
            strcpy(tempId, padded);
        }

        if (!validateId(tempId)) continue;

        if (findComponent(tempId, NULL, NULL)) {
            printf("ID sudah ada!\n");
            pause();
            return;
        }

        strcpy(comp.id, tempId);
        break;
    }

    printf("Nama komponen: ");
    fgets(comp.name, sizeof(comp.name), stdin);
    comp.name[strcspn(comp.name, "\n")] = '\0';

    do {
        printf("Stok: ");
        scanf("%d", &comp.stock);
        clearInput();
    } while (comp.stock < 0 && printf("Stok tidak boleh negatif!\n"));

    do {
        printf("Harga: ");
        scanf("%f", &comp.price);
        clearInput();
    } while (comp.price < 0 && printf("Harga tidak boleh negatif!\n"));

    saveComponent(&comp, -1);
    printf("Data berhasil ditambahkan!\n");
    pause();
}

void editComponent() {
    char id[ID_LENGTH + 1];
    Component comp;
    long position;
    
    printf("ID komponen yang akan diubah: ");
    scanf("%s", id);
    clearInput();
    
    if (!findComponent(id, &comp, &position)) {
        printf("Komponen tidak ditemukan!\n");
        pause();
        return;
    }
    
    printf("\nData saat ini:\n");
    printf("ID: %s | Nama: %s | Stok: %d | Harga: %.2f\n", comp.id, comp.name, comp.stock, comp.price);
    printf("\nData baru (Enter untuk skip):\n");
    
    char input[100];
    printf("Nama [%s]: ", comp.name);
    fgets(input, sizeof(input), stdin);
    if (strlen(input) > 1) {
        input[strcspn(input, "\n")] = '\0';
        strcpy(comp.name, input);
    }
    
    printf("Stok [%d]: ", comp.stock);
    fgets(input, sizeof(input), stdin);
    int newStock;
    if (sscanf(input, "%d", &newStock) == 1 && newStock >= 0) {
        comp.stock = newStock;
    }
    
    printf("Harga [%.2f]: ", comp.price);
    fgets(input, sizeof(input), stdin);
    float newPrice;
    if (sscanf(input, "%f", &newPrice) == 1 && newPrice >= 0) {
        comp.price = newPrice;
    }
    
    saveComponent(&comp, position);
    printf("Data berhasil diubah!\n");
    pause();
}

void displayComponents(int showIndex) {
    Component *components = NULL;
    int count = loadAllComponents(&components);

    if (count == 0) {
        printf("Tidak ada data!\n");
        pause();
        return;
    }

    if (showIndex) {
        sortComponents(components, count);
    }
    else {
        sortByName(components, count);
    }

    float totalAsset = 0;
    for (int i = 0; i < count; i++) {
        totalAsset += components[i].stock * components[i].price;
    }

    printf("\n%-8s %-25s %-8s %-10s\n", "ID", "Nama", "Stok", "Harga");
    printf("%s\n", "====================================================================");

    for (int i = 0; i < count; i++) {
        printf("%-8s %-25s %-8d %-10.2f\n", components[i].id, components[i].name, components[i].stock, components[i].price);

        if ((i + 1) % PAGE_SIZE == 0 && i < count - 1) {
            printf("\n--- Enter untuk lanjut ---");
            getchar();
        }
    }

    printf("\nTotal aset: Rp %.2f\n", totalAsset);
    free(components);
    pause();
}


void deleteComponentMenu() {
    char id[ID_LENGTH + 1];
    
    printf("ID komponen yang akan dihapus: ");
    scanf("%s", id);
    clearInput();
    
    if (deleteComponent(id)) {
        printf("Komponen %s berhasil dihapus!\n", id);
    } 
    else {
        printf("Komponen %s tidak ditemukan!\n", id);
    }
    pause();
}

void tampilMenu(int posisi) {
    system("cls||clear");
    printf("=== SISTEM MANAJEMEN KOMPONEN KENDARAAN ===\n");

    const char *menu[] = {
        "Tambah Komponen",
        "Ubah Komponen",
        "Lihat Semua (dengan index)",
        "Lihat Semua (tanpa index)",
        "Hapus Komponen",
        "Keluar"
    };

    for (int i = 0; i < 6; i++) {
        if (i == posisi) {
            printf("> %d. %s\n", i + 1, menu[i]);
        }
        else {
            printf("  %d. %s\n", i + 1, menu[i]);
        }
    }

    printf("\nGunakan W (naik), S (turun), Enter (pilih)");
}

int main() {
    int posisi = 0;
    char input;

    while (1) {
        tampilMenu(posisi);
        input = getch();

        if (input == 'w' || input == 'W') {
            posisi = (posisi - 1 + 6) % 6;
        } 
        else if (input == 's' || input == 'S') {
            posisi = (posisi + 1) % 6;
        } 
        else if (input == '\r') {
            switch (posisi) {
                case 0: addComponent(); break;
                case 1: editComponent(); break;
                case 2: displayComponents(1); break;
                case 3: displayComponents(0); break;
                case 4: deleteComponentMenu(); break;
                case 5: printf("\nKeluar...\n"); return 0;
            }
        }
    }

    return 0;
}
