FIND := $(shell which gfind 2>/dev/null || which find)
JAVA  := $(shell $(FIND) src -type f -name *.java)
CLASS := $(patsubst src/%.java, build/%.class, $(JAVA))

JAVACOPT := -Xlint:deprecation

all: build/.create $(CLASS) build/output.jar
build/.create:
	@mkdir -p build
	@touch $@

$(CLASS): build/%.class: src/%.java
	@echo "[JAVAC] $<"
	@javac $(JAVACOPT) -cp 'src:deps/genclass.jar' $< -d build

build/MANIFEST.MF:
	@echo "[CREATE] MANIFEST.MF"
	@echo "Manifest-Version: 1.0" > $@
	@echo "Main-Class: sd.main.Main" >> $@
	@echo "Class-Path: deps/genclass.jar" >> $@

build/deps:
	@echo "[MKDIR] $@"
	@mkdir -p $@
	@cp deps/genclass.jar $@/

build/output.jar: $(CLASS) build/MANIFEST.MF build/deps
	@echo "[JAR] $@"
	@jar cfm $@ build/MANIFEST.MF -C build .

run: build/output.jar
	@java -jar $<

.PHONY: clean
clean:
	@echo "[RM] build"
	@rm -rf build
	@echo "[RM] output.jar"
	@rm -rf build/output.jar
