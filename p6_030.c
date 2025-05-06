#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>

/*
 * Program ini mengimplementasikan Masalah Lima Filsuf Makan
 * berdasarkan model Petri Net yang telah dioptimasi oleh saya.
 * 
 * Aspek-aspek kunci model:
 * 1. Setiap filsuf membutuhkan dua sumber daya (garpu) untuk makan
 * 2. Filsuf mengambil kedua sumber daya secara bersamaan (operasi atomik)
 * 3. Filsuf melepaskan kedua sumber daya secara bersamaan (operasi atomik)
 * 4. Beberapa filsuf dapat makan secara bersamaan jika sumber daya tersedia
 * 5. Model ini mencegah deadlock melalui pengelolaan sumber daya yang mengembalikan atau mengambil langsung 2 resource seklaigus
 */

// Jumlah filsuf/sumber daya
#define N 5

// Status filsuf (sesuai dengan places di Petri Net)
#define THINKING 0  // Filsuf 1-5 dalam Petri Net
#define HUNGRY 1    // Status transisi (tidak secara eksplisit dalam Petri Net)
#define EATING 2    // Eating 1-5 dalam Petri Net

// Variabel global
pthread_mutex_t mutex;              // Mutex untuk mengamankan akses ke sumber daya
pthread_cond_t cond[N];             // Variabel kondisi untuk setiap filsuf
int state[N];                       // Status setiap 
pthread_t philosophers[N];          // Thread untuk filsuf
int resources[N] = {1, 1, 1, 1, 1}; // Sumber daya awal (semua tersedia = 1 token di Resource 1-5)
char *names[N] = {"Filsuf 1", "Filsuf 2", "Filsuf 3", "Filsuf 4", "Filsuf 5"};

// Prototipe fungsi
void *philosopher(void *arg);
void think(int id);
void take_resources(int id);
void eat(int id);
void put_resources(int id);
int left(int id);
int right(int id);
void test(int id);
void print_state();

int main() {
    int i;
    
    // Inisialisasi pembangkit angka acak
    srand(time(NULL));
    
    // Inisialisasi mutex dan variabel kondisi
    pthread_mutex_init(&mutex, NULL);
    
    for (i = 0; i < N; i++) {
        pthread_cond_init(&cond[i], NULL);
        state[i] = THINKING;  // Semua filsuf mulai berpikir (marking awal)
    }
    
    printf("Masalah Lima Filsuf Makan\n");
    printf("--------------------------------\n");
    
    // Buat thread filsuf
    for (i = 0; i < N; i++) {
        int *id = malloc(sizeof(int));
        *id = i;
        pthread_create(&philosophers[i], NULL, philosopher, id);
    }
    
    // Gabungkan thread filsuf - meskipun tidak akan pernah mencapai titik ini karena loop tak terbatas
    for (i = 0; i < N; i++) {
        pthread_join(philosophers[i], NULL);
    }
    
    // Pembersihan - meskipun tidak akan pernah mencapai titik ini karena loop tak terbatas
    pthread_mutex_destroy(&mutex);
    for (i = 0; i < N; i++) {
        pthread_cond_destroy(&cond[i]);
    }
    
    return 0;
}

// Fungsi thread filsuf - merepresentasikan siklus utama dalam Petri Net
void *philosopher(void *arg) {
    int id = *(int*)arg;
    free(arg);
    
    // Loop selamanya - ini merepresentasikan siklus dalam Petri Net
    while (1) {
        think(id);             // Filsuf berpikir (tempat Filsuf 1-5)
        take_resources(id);    // Filsuf mencoba mengambil kedua sumber daya (transisi dari Filsuf ke Eating)
        eat(id);               // Filsuf makan (tempat Eating 1-5)
        put_resources(id);     // Filsuf melepaskan kedua sumber daya (transisi Return Resource 1-5)
    }
    
    return NULL;
}

// Filsuf berpikir selama periode acak
void think(int id) {
    printf("%s sedang berpikir\n", names[id]);
    sleep(1 + rand() % 3);  // Berpikir selama 1-3 detik
}

// Filsuf mencoba mengambil kedua sumber daya
// Ini sesuai dengan transisi dari Filsuf ke Eating dalam Petri Net
void take_resources(int id) {
    pthread_mutex_lock(&mutex);  // Lock mutex untuk mengamankan akses ke sumber daya
    
    // Ubah status menjadi HUNGRY
    state[id] = HUNGRY;
    printf("\n%s lapar dan mencoba mengambil Sumber Daya %d dan Sumber Daya %d\n", 
           names[id], left(id) + 1, id + 1);
    print_state();
    
    // Coba ambil kedua sumber daya
    // Ini adalah pemicu transisi jika kedua tempat input (sumber daya) memiliki token
    test(id);
    
    // Jika tidak dapat mengambil kedua sumber daya, tunggu
    // Ini merepresentasikan sifat pemblokiran transisi dalam Petri Net ketika input tidak tersedia
    if (state[id] != EATING) {
        printf("\n%s sedang menunggu sumber daya...\n", names[id]);
        pthread_cond_wait(&cond[id], &mutex);
    }
    
    pthread_mutex_unlock(&mutex);  // Keluar bagian kritis
}

// Filsuf makan selama periode acak
// Ini sesuai dengan berada di tempat Eating 1-5 dalam Petri Net
void eat(int id) {
    printf("\n%s sedang makan menggunakan Sumber Daya %d dan Sumber Daya %d\n", 
           names[id], left(id) + 1, id + 1);
    print_state();
    sleep(1 + rand() % 3);  // Makan selama 1-3 detik
}

// Filsuf meletakkan kembali kedua sumber daya
// Ini sesuai dengan transisi Return Resource 1-5 dalam Petri Net
void put_resources(int id) {
    pthread_mutex_lock(&mutex);  // Masuk bagian kritis
    
    // Ubah status kembali menjadi THINKING
    state[id] = THINKING;
    printf("\n%s meletakkan kembali Sumber Daya %d dan Sumber Daya %d\n", 
           names[id], left(id) + 1, id + 1);
    
    // Membuat sumber daya tersedia - mengembalikan token ke tempat Resource
    resources[left(id)] = 1;
    resources[id] = 1;
    
    // Periksa apakah tetangga sekarang dapat makan (ini adalah efek kaskade dari ketersediaan token)
    test(left(id));
    test(right(id));
    
    print_state();
    pthread_mutex_unlock(&mutex);  // Keluar bagian kritis
}

// Mendapatkan indeks sumber daya kiri - mengimplementasikan pengaturan sumber daya melingkar
int left(int id) {
    return (id + N - 1) % N;
}

// Mendapatkan indeks sumber daya kanan - mengimplementasikan pengaturan sumber daya melingkar
int right(int id) {
    return (id + 1) % N;
}

// Memeriksa apakah filsuf dapat makan
// Ini memeriksa apakah transisi dari Filsuf ke Eating dapat terpicu
void test(int id) {
    // Jika lapar dan kedua sumber daya tersedia (memiliki token)
    if (state[id] == HUNGRY && resources[left(id)] == 1 && resources[id] == 1) {
        // Ubah status menjadi EATING (token berpindah ke tempat Eating)
        state[id] = EATING;
        
        // Ambil kedua sumber daya (hapus token dari tempat Resource)
        resources[left(id)] = 0;
        resources[id] = 0;
        
        // Sinyal filsuf bahwa mereka dapat makan
        pthread_cond_signal(&cond[id]);
    }
}

// Cetak status saat ini dari sumber daya dan filsuf
void print_state() {
    int i;
    
    // Tampilkan status sumber daya (ketersediaan token)
    printf("Sumber Daya: ");
    for (i = 0; i < N; i++) {
        printf("%d ", resources[i]);
    }
    
    // Tampilkan status filsuf
    printf("\nSTATE Para Filsuf: \n");
    for (i = 0; i < N; i++) {
        char *state_str;
        switch(state[i]) {
            case THINKING: state_str = "BERPIKIR"; break;
            case HUNGRY: state_str = "LAPAR"; break;
            case EATING: state_str = "MAKAN"; break;
            default: state_str = "TIDAK DIKETAHUI";
        }
        printf("%s: %s, \n", names[i], state_str);
        // Enter untuk melanjutkan ke baris berikutnya
        getchar(); // Tunggu input pengguna untuk melanjutkan
    }
    printf("\n");
}