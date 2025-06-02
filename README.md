# Pengontrol Pompa Air Semi Otomatis dengan Timer dan Pengaturan Kecepatan PWM

Proyek ini adalah sistem kontrol semi otomatis untuk pompa air yang dilengkapi dengan fitur timer dan pengaturan kecepatan pompa menggunakan PWM, serta penyimpanan pengaturan ke EEPROM. Sistem ini dirancang untuk memudahkan pengguna dalam mengatur durasi pengisian dan kekuatan aliran pompa air.

[https://github.com/hxndani/filler-machine-semi-automate/blob/39c58312a4cee7119cf76e0a91b37a0005bb82d4/WhatsApp%20Image%202025-05-31%20at%2014.37.15%20(2).jpeg]

# Pinout dan Fungsi
PB11	SSR Pin: Mengontrol Solid State Relay untuk membuka/menutup keran solenoid.
PB1	MOSFET Pin: Output PWM untuk mengontrol kecepatan pompa air.
PB15	Button Power Pin: Tombol untuk memulai atau menghentikan proses pengisian.
PB14	Button Plus Pin: Tombol untuk menambah nilai timer atau kecepatan pompa.
PB13	Button Min Pin: Tombol untuk mengurangi nilai timer atau kecepatan pompa.
PB12	Button Save Pin: Tombol untuk menyimpan pengaturan timer atau kecepatan pompa ke EEPROM.
PB3	Button Menu Pin: Tombol untuk beralih antara menu pengaturan timer dan kecepatan pompa.
I2C (SDA, SCL)	LCD I2C: Untuk menampilkan informasi dan pengaturan pada LCD 16x2.

# Fitur Utama
- Kontrol Otomatis: Memulai dan menghentikan proses pengisian air secara otomatis berdasarkan timer yang telah ditentukan.
- Pengaturan Timer: Pengguna dapat mengatur durasi pengisian air mulai dari 0 hingga 3600 detik (1 jam) dengan resolusi 0,01 detik.
- Pengaturan Kecepatan Pompa (PWM): Mengatur kekuatan atau kecepatan aliran pompa air dari 0% hingga 100%.
- Penyimpanan Otomatis (EEPROM): Pengaturan timer dan kecepatan pompa yang telah disimpan akan tetap tersimpan meskipun daya dimatikan.
- Tampilan LCD Interaktif: Menampilkan waktu hitung mundur saat pengisian, serta nilai pengaturan saat ini dan nilai yang tersimpan.
- Mode Pengaturan: Beralih antara mode pengaturan timer dan kecepatan pompa melalui tombol menu.
- Indikator "Saved!": Menampilkan pesan singkat di LCD setelah pengaturan berhasil disimpan.

# Cara Menggunakan Alat
1. Nyalakan perangkat: LCD akan menampilkan menu pengaturan.
2. Pilih Menu: Tekan Tombol Menu (PB3) untuk beralih antara pengaturan Timer (Set T) dan Kecepatan Pompa (Set Spd).
3. Atur Nilai:
      - Pada menu Timer: Gunakan Tombol Plus (PB14) dan Tombol Min (PB13) untuk menambah atau mengurangi durasi pengisian.
      - Pada menu Kecepatan Pompa: Gunakan Tombol Plus (PB14) dan Tombol Min (PB13) untuk menambah atau mengurangi persentase kecepatan PWM.
4. Simpan Pengaturan: Setelah mengatur nilai yang diinginkan, tekan Tombol Save (PB12) untuk menyimpan pengaturan ke memori EEPROM. Pesan "Saved!" akan muncul sebentar di       LCD.
5. Mulai/Hentikan Pengisian: Tekan Tombol Power (PB15) untuk memulai proses pengisian air. Pompa akan menyala dan keran akan terbuka sesuai dengan pengaturan yang tersimpan.    Tekan kembali Tombol Power untuk menghentikan proses pengisian kapan saja.
6. Mode Pengisian: Selama pengisian, LCD akan menampilkan hitung mundur timer dan persentase kecepatan pompa saat ini.


# Pengaturan Parameter Melalui Serial Monitor (Debugging)
Meskipun program ini dirancang untuk beroperasi secara mandiri melalui tombol dan LCD, Anda dapat memantau beberapa parameter penting melalui Serial Monitor untuk tujuan debugging:

1. Buka Serial Monitor di Arduino IDE Anda (pastikan baud rate diatur ke 9600).
2. Saat program dimulai, Anda akan melihat nilai Timer yang tersimpan (ms) dan Kecepatan Motor PWM yang dibaca dari EEPROM.
3. Setiap kali Anda menyimpan pengaturan timer atau kecepatan motor menggunakan tombol Save, nilai yang disimpan akan dicetak ke Serial Monitor:
    - Timer saved: [nilai timer dalam ms]
    - Speed Motor saved: [nilai PWM]
4. Ketika proses pengisian dimulai atau dihentikan, Anda juga akan melihat pesan di Serial Monitor:
    - Filling started with timer: [nilai timer dalam ms] ms, Speed: [nilai PWM] PWM
    - Filling stopped
   
Serial Monitor berguna untuk memverifikasi bahwa nilai-nilai disimpan dan dibaca dengan benar dari EEPROM, serta untuk memantau status operasi sistem.
