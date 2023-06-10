Алгоритм Хаффмана для сжатия файлов.

Сборка из подкаталога:
cmake ..


Сжатие файла myfile.txt в архив result.bin:
./huffman -c -f myfile.txt -o result.bin

Распаковка архива result.bin обратно в текстовый файл myfile_new.txt:
./huffman -u -f result.bin -o myfile_new.txt


Запуск тестов:
./huffman_tests
