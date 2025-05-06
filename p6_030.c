/**
 * Masalah Lima Filsuf Makan - Implementasi berdasarkan model Petri Net yang dioptimalkan
 * Nama: Roy Aziz Barera
 * NIM: 221524030
 * 
 * Program ini mengimplementasikan Masalah Lima Filsuf Makan menggunakan Pthreads
 * berdasarkan model Petri Net yang dioptimalkan dari tugas sebelumnya.
 * Dengan tambahan fitur untuk mode manual dan otomatis.
 */

 #include <stdio.h>
 #include <stdlib.h>
 #include <pthread.h>
 #include <unistd.h>
 #include <time.h>
 #include <conio.h> // Untuk getch()
 #include <stdbool.h>
 
 #define NUM_PHILOSOPHERS 5
 #define THINKING_TIME 2
 #define EATING_TIME 3
 #define ITERATION_COUNT 3
 #define MIN_AUTO_INTERVAL 3
 #define MAX_AUTO_INTERVAL 5
 
 // Mutex untuk setiap sumber daya/sumpit (sesuai dengan Resource 1-5 dalam model)
 pthread_mutex_t resources[NUM_PHILOSOPHERS];
 
 // Status untuk setiap filsuf (0: berpikir, 1: mencoba makan, 2: sedang makan)
 int philosopher_status[NUM_PHILOSOPHERS] = {0};
 
 // Mutex untuk melindungi akses ke status filsuf
 pthread_mutex_t status_mutex;
 
 // Variabel untuk mode otomatis
 bool auto_mode = false;
 bool program_running = true;
 int auto_interval_min = MIN_AUTO_INTERVAL;
 int auto_interval_max = MAX_AUTO_INTERVAL;
 pthread_t auto_thread;
 
 // Thread untuk setiap filsuf dalam mode konkuren
 pthread_t philosopher_threads[NUM_PHILOSOPHERS];
 bool concurrent_mode = false;
 
 // Prototipe fungsi
 void think(int philosopher_id);
 void eat(int philosopher_id);
 void pickup_resources(int philosopher_id);
 void return_resources(int philosopher_id);
 void print_status();
 void *auto_mode_thread(void *arg);
 void manual_mode();
 void auto_mode_control();
 void concurrent_mode_control();
 void *philosopher_thread_function(void *arg);
 void philosopher_action(int philosopher_id);
 
 /**
  * Fungsi utama
  * Menginisialisasi sumber daya dan menjalankan mode yang dipilih
  */
 int main() {
     int i;
     int mode_choice;
     
     // Inisialisasi seed acak
     srand(time(NULL));
     
     printf("Masalah Lima Filsuf Makan - Berdasarkan Model Petri Net yang Dioptimalkan\n");
     printf("------------------------------------------------------------\n");
     
     // Inisialisasi mutex untuk status
     if (pthread_mutex_init(&status_mutex, NULL) != 0) {
         printf("Gagal menginisialisasi mutex untuk status\n");
         return 1;
     }
     
     // Inisialisasi sumber daya/sumpit
     for (i = 0; i < NUM_PHILOSOPHERS; i++) {
         if (pthread_mutex_init(&resources[i], NULL) != 0) {
             printf("Gagal menginisialisasi mutex untuk sumber daya %d\n", i);
             return 1;
         }
     }
     
     printf("Pilih mode:\n");
     printf("1. Mode Manual (Pilih filsuf untuk makan)\n");
     printf("2. Mode Otomatis (Filsuf makan secara bergantian sesuai interval waktu)\n");
     printf("3. Mode Konkuren (Semua filsuf mencoba makan bersamaan jika memungkinkan)\n");
     printf("Pilihan Anda: ");
     scanf("%d", &mode_choice);
     
     if (mode_choice == 1) {
         // Mode Manual
         manual_mode();
     } else if (mode_choice == 2) {
         // Mode Otomatis
         auto_mode_control();
     } else if (mode_choice == 3) {
         // Mode Konkuren
         concurrent_mode_control();
     } else {
         printf("Pilihan tidak valid!\n");
     }
     
     // Menghapus mutex
     for (i = 0; i < NUM_PHILOSOPHERS; i++) {
         pthread_mutex_destroy(&resources[i]);
     }
     pthread_mutex_destroy(&status_mutex);
     
     printf("\nProgram selesai.\n");
     
     return 0;
 }
 
 /**
  * Mode manual: memungkinkan pengguna memilih filsuf mana yang akan makan
  */
 void manual_mode() {
     int choice;
     
     printf("\nMode Manual\n");
     printf("------------\n");
     
     while (1) {
         print_status();
         
         printf("\nMenu:\n");
         printf("1-5: Pilih filsuf untuk makan\n");
         printf("0: Keluar\n");
         printf("Pilihan Anda: ");
         scanf("%d", &choice);
         
         if (choice >= 1 && choice <= 5) {
             philosopher_action(choice - 1);
         } else if (choice == 0) {
             break;
         } else {
             printf("Pilihan tidak valid!\n");
         }
     }
 }
 
 /**
  * Mode otomatis: filsuf akan makan secara otomatis dengan interval waktu tertentu
  */
 void auto_mode_control() {
     int choice;
     
     printf("\nMode Otomatis\n");
     printf("-------------\n");
     printf("Masukkan interval waktu minimum (detik): ");
     scanf("%d", &auto_interval_min);
     printf("Masukkan interval waktu maksimum (detik): ");
     scanf("%d", &auto_interval_max);
     
     auto_mode = true;
     
     // Membuat thread untuk mode otomatis
     pthread_create(&auto_thread, NULL, auto_mode_thread, NULL);
     
     printf("\nMode otomatis dijalankan dengan interval %d-%d detik\n", auto_interval_min, auto_interval_max);
     printf("Tekan '0' untuk kembali ke menu utama\n");
     
     while (1) {
         if (_kbhit()) {
             choice = _getch() - '0';
             if (choice == 0) {
                 program_running = false;
                 auto_mode = false;
                 pthread_join(auto_thread, NULL);
                 break;
             }
         }
         sleep(1);
     }
 }
 
 /**
  * Mode konkuren: semua filsuf mencoba makan secara bersamaan jika memungkinkan
  */
 void concurrent_mode_control() {
     int i, choice;
     
     printf("\nMode Konkuren\n");
     printf("-------------\n");
     printf("Pada mode ini, filsuf akan mencoba makan ketika mereka memiliki kesempatan\n");
     printf("Beberapa filsuf dapat makan secara bersamaan jika kondisi memungkinkan\n");
     
     concurrent_mode = true;
     program_running = true;
     
     // Membuat thread untuk setiap filsuf
     for (i = 0; i < NUM_PHILOSOPHERS; i++) {
         int *id = malloc(sizeof(int));
         *id = i;
         pthread_create(&philosopher_threads[i], NULL, philosopher_thread_function, (void*)id);
     }
     
     printf("\nMode konkuren dijalankan. Tekan '0' untuk kembali ke menu utama\n");
     
     while (1) {
         if (_kbhit()) {
             choice = _getch() - '0';
             if (choice == 0) {
                 program_running = false;
                 concurrent_mode = false;
                 
                 // Menunggu semua thread filsuf selesai
                 for (i = 0; i < NUM_PHILOSOPHERS; i++) {
                     pthread_join(philosopher_threads[i], NULL);
                 }
                 
                 break;
             }
         }
         
         // Tampilkan status setiap beberapa detik
         print_status();
         sleep(2);
     }
 }
 
 /**
  * Thread untuk filsuf dalam mode konkuren
  */
 void *philosopher_thread_function(void *arg) {
     int philosopher_id = *((int *)arg);
     free(arg);
     
     while (concurrent_mode && program_running) {
         // Filsuf berpikir untuk beberapa saat
         think(philosopher_id);
         
         // Mencoba untuk makan
         pthread_mutex_lock(&status_mutex);
         philosopher_status[philosopher_id] = 1; // mencoba makan
         pthread_mutex_unlock(&status_mutex);
         
         // Coba ambil sumpit dengan algoritma yang mencegah deadlock
         int can_eat = 1;
         int first_resource, second_resource;
         
         // Gunakan strategi untuk mencegah deadlock: filosofer genap mengambil sumpit kiri dulu, ganjil mengambil kanan dulu
         if (philosopher_id % 2 == 0) {
             first_resource = philosopher_id;
             second_resource = (philosopher_id + 1) % NUM_PHILOSOPHERS;
         } else {
             second_resource = (philosopher_id + 1) % NUM_PHILOSOPHERS;
             first_resource = philosopher_id;
         }
         
         // Coba ambil sumpit pertama
         if (pthread_mutex_trylock(&resources[first_resource]) != 0) {
             can_eat = 0;
             printf("Filsuf %d tidak bisa mengambil sumpit pertama (Sumber Daya %d)\n", philosopher_id + 1, first_resource + 1);
         } else {
             printf("Filsuf %d mengambil sumpit pertama (Sumber Daya %d)\n", philosopher_id + 1, first_resource + 1);
             
             // Coba ambil sumpit kedua
             if (pthread_mutex_trylock(&resources[second_resource]) != 0) {
                 can_eat = 0;
                 pthread_mutex_unlock(&resources[first_resource]);
                 printf("Filsuf %d tidak bisa mengambil sumpit kedua (Sumber Daya %d), mengembalikan sumpit pertama\n", philosopher_id + 1, second_resource + 1);
             } else {
                 printf("Filsuf %d mengambil sumpit kedua (Sumber Daya %d)\n", philosopher_id + 1, second_resource + 1);
             }
         }
         
         if (can_eat) {
             // Filsuf bisa makan
             pthread_mutex_lock(&status_mutex);
             philosopher_status[philosopher_id] = 2; // sedang makan
             pthread_mutex_unlock(&status_mutex);
             
             // Makan
             eat(philosopher_id);
             
             // Kembalikan sumpit
             pthread_mutex_unlock(&resources[second_resource]);
             printf("Filsuf %d mengembalikan sumpit kedua (Sumber Daya %d)\n", philosopher_id + 1, second_resource + 1);
             
             pthread_mutex_unlock(&resources[first_resource]);
             printf("Filsuf %d mengembalikan sumpit pertama (Sumber Daya %d)\n", philosopher_id + 1, first_resource + 1);
             
             pthread_mutex_lock(&status_mutex);
             philosopher_status[philosopher_id] = 0; // kembali berpikir
             pthread_mutex_unlock(&status_mutex);
         } else {
             // Filsuf tidak bisa makan, kembali berpikir
             pthread_mutex_lock(&status_mutex);
             philosopher_status[philosopher_id] = 0;
             pthread_mutex_unlock(&status_mutex);
             
             // Tunggu sebentar sebelum mencoba lagi
             sleep(1);
         }
     }
     
     return NULL;
 }
 
 /**
  * Thread untuk mode otomatis
  */
 void *auto_mode_thread(void *arg) {
     while (auto_mode && program_running) {
         // Pilih filsuf secara acak
         int philosopher_id = rand() % NUM_PHILOSOPHERS;
         
         // Jalankan aksi filsuf
         philosopher_action(philosopher_id);
         
         // Tunggu interval waktu acak
         int wait_time = auto_interval_min + rand() % (auto_interval_max - auto_interval_min + 1);
         sleep(wait_time);
     }
     
     return NULL;
 }
 
 /**
  * Menjalankan aksi untuk filsuf tertentu
  */
 void philosopher_action(int philosopher_id) {
     pthread_mutex_lock(&status_mutex);
     int status = philosopher_status[philosopher_id];
     pthread_mutex_unlock(&status_mutex);
     
     if (status == 0) {
         // Jika filsuf sedang berpikir, coba makan
         printf("\nFilsuf %d mencoba untuk makan\n", philosopher_id + 1);
         
         pthread_mutex_lock(&status_mutex);
         philosopher_status[philosopher_id] = 1; // mencoba makan
         pthread_mutex_unlock(&status_mutex);
         
         // Coba ambil sumpit
         int can_eat = 1;
         
         int first_resource = philosopher_id;
         int second_resource = (philosopher_id + 1) % NUM_PHILOSOPHERS;
         
         if (pthread_mutex_trylock(&resources[first_resource]) != 0) {
             can_eat = 0;
             printf("Filsuf %d tidak bisa mengambil sumpit pertama (Sumber Daya %d)\n", philosopher_id + 1, first_resource + 1);
         } else {
             printf("Filsuf %d mengambil sumpit pertama (Sumber Daya %d)\n", philosopher_id + 1, first_resource + 1);
             
             if (pthread_mutex_trylock(&resources[second_resource]) != 0) {
                 can_eat = 0;
                 pthread_mutex_unlock(&resources[first_resource]);
                 printf("Filsuf %d tidak bisa mengambil sumpit kedua (Sumber Daya %d), mengembalikan sumpit pertama\n", philosopher_id + 1, second_resource + 1);
             } else {
                 printf("Filsuf %d mengambil sumpit kedua (Sumber Daya %d)\n", philosopher_id + 1, second_resource + 1);
             }
         }
         
         if (can_eat) {
             // Filsuf bisa makan
             pthread_mutex_lock(&status_mutex);
             philosopher_status[philosopher_id] = 2; // sedang makan
             pthread_mutex_unlock(&status_mutex);
             
             int eat_time = rand() % EATING_TIME + 1;
             printf("Filsuf %d sedang makan selama %d detik\n", philosopher_id + 1, eat_time);
             sleep(eat_time);
             
             // Kembalikan sumpit
             pthread_mutex_unlock(&resources[second_resource]);
             printf("Filsuf %d mengembalikan sumpit kedua (Sumber Daya %d)\n", philosopher_id + 1, second_resource + 1);
             
             pthread_mutex_unlock(&resources[first_resource]);
             printf("Filsuf %d mengembalikan sumpit pertama (Sumber Daya %d)\n", philosopher_id + 1, first_resource + 1);
             
             pthread_mutex_lock(&status_mutex);
             philosopher_status[philosopher_id] = 0; // kembali berpikir
             pthread_mutex_unlock(&status_mutex);
         } else {
             // Filsuf tidak bisa makan, kembali berpikir
             pthread_mutex_lock(&status_mutex);
             philosopher_status[philosopher_id] = 0;
             pthread_mutex_unlock(&status_mutex);
         }
     } else if (status == 2) {
         // Jika filsuf sedang makan, kembalikan sumpit
         printf("\nFilsuf %d menyelesaikan makan dan mengembalikan sumpit\n", philosopher_id + 1);
         
         int first_resource = philosopher_id;
         int second_resource = (philosopher_id + 1) % NUM_PHILOSOPHERS;
         
         pthread_mutex_unlock(&resources[second_resource]);
         printf("Filsuf %d mengembalikan sumpit kedua (Sumber Daya %d)\n", philosopher_id + 1, second_resource + 1);
         
         pthread_mutex_unlock(&resources[first_resource]);
         printf("Filsuf %d mengembalikan sumpit pertama (Sumber Daya %d)\n", philosopher_id + 1, first_resource + 1);
         
         pthread_mutex_lock(&status_mutex);
         philosopher_status[philosopher_id] = 0; // kembali berpikir
         pthread_mutex_unlock(&status_mutex);
     } else {
         printf("\nFilsuf %d sedang mencoba mengambil sumpit\n", philosopher_id + 1);
     }
 }
 
 /**
  * Menampilkan status semua filsuf
  */
 void print_status() {
     int i;
     
     printf("\nSTATUS FILSUF:\n");
     printf("--------------\n");
     
     pthread_mutex_lock(&status_mutex);
     for (i = 0; i < NUM_PHILOSOPHERS; i++) {
         printf("Filsuf %d: ", i + 1);
         switch (philosopher_status[i]) {
             case 0:
                 printf("Berpikir");
                 break;
             case 1:
                 printf("Mencoba makan");
                 break;
             case 2:
                 printf("Sedang makan");
                 break;
             default:
                 printf("Status tidak diketahui");
         }
         printf("\n");
     }
     pthread_mutex_unlock(&status_mutex);
 }
 
 // Fungsi bawaan yang tetap dipertahankan
 void think(int philosopher_id) {
     int think_time = rand() % THINKING_TIME + 1;
     printf("Filsuf %d sedang berpikir selama %d detik\n", philosopher_id + 1, think_time);
     sleep(think_time);
 }
 
 void eat(int philosopher_id) {
     int eat_time = rand() % EATING_TIME + 1;
     printf("Filsuf %d sedang makan selama %d detik\n", philosopher_id + 1, eat_time);
     sleep(eat_time);
 }
 
 void pickup_resources(int philosopher_id) {
     printf("Filsuf %d mencoba mengambil sumpit\n", philosopher_id + 1);
     
     int first_resource = philosopher_id;
     int second_resource = (philosopher_id + 1) % NUM_PHILOSOPHERS;
     
     pthread_mutex_lock(&resources[first_resource]);
     printf("Filsuf %d mengambil sumpit pertama (Sumber Daya %d)\n", philosopher_id + 1, first_resource + 1);
     
     sleep(1);  // Sedikit delay
     
     pthread_mutex_lock(&resources[second_resource]);
     printf("Filsuf %d mengambil sumpit kedua (Sumber Daya %d)\n", philosopher_id + 1, second_resource + 1);
 }
 
 void return_resources(int philosopher_id) {
     int first_resource = philosopher_id;
     int second_resource = (philosopher_id + 1) % NUM_PHILOSOPHERS;
     
     pthread_mutex_unlock(&resources[second_resource]);
     printf("Filsuf %d mengembalikan sumpit kedua (Sumber Daya %d)\n", philosopher_id + 1, second_resource + 1);
     
     pthread_mutex_unlock(&resources[first_resource]);
     printf("Filsuf %d mengembalikan sumpit pertama (Sumber Daya %d)\n", philosopher_id + 1, first_resource + 1);
 }