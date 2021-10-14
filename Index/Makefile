CC = g++
FLAGS = -Wall -Werror -std=c++17 -fsanitize=address

define FILES 
data/lenta_ru_159b9f4b-972b-48b1-8ec3-44fbd6be33c4_01 \
data/lenta_ru_4deb864d-3c46-45e6-85f4-a7ff7544a3fb_01 \
data/lenta_ru_6398c7e2-16da-40d2-8923-95f65aaaeb07_01 \
data/lenta_ru_80e74243-83da-4367-8ae3-fe38d333f283_01 \
data/lenta_ru_aa5a1ef9-6ca4-4dc7-890f-308d4d62db59_01 \
data/lenta_ru_b6838708-1aa9-496f-bf88-e277374f93a8_01 \
data/lenta_ru_b81aa623-ba55-43dc-b3c5-47ae2253ad27_01 \
data/lenta_ru_d1f7e910-b5f1-4719-b724-090093e143fe_01
endef


all: index

test: index
	./$< --test $(FILES)

simpleidx: index
	./$< --simpleidx $(FILES)

simpleidx_qtree: index
	./$< --simpleidx_qtree $(FILES)

compressidx_qtree: index
	./$< --compressidx_qtree $(FILES)

index: main.o Index.o SimpleQuery.o QTree.o Test.o
	$(CC) $(FLAGS) $^ -o $@

main.o: main.cpp Test.hpp QTree.hpp SimpleQuery.hpp Index.hpp
	$(CC) $(FLAGS) -c $< -o $@

Test.o: Test.cpp Test.hpp
	$(CC) $(FLAGS) -c $< -o $@

QTree.o: QTree.cpp QTree.hpp
	$(CC) $(FLAGS) -c $< -o $@

SimpleQuery.o: SimpleQuery.cpp SimpleQuery.hpp
	$(CC) $(FLAGS) -c $< -o $@

Index.o: Index.cpp Index.hpp
	$(CC) $(FLAGS) -c $< -o $@

clean:
	rm -rf *.o index