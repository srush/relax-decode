JAVAC=		javac
JAVA=		java

HOME_DIR=	..
MODEL_DIR=	../model
SAMPLES_DIR=	../samples
SRC_DIR=	crf/tagger

JAR_FILE=	crftagger.jar

CLASSPATH=	.

all:
	$(JAVAC) -classpath $(CLASSPATH) $(SRC_DIR)/CRFTagger.java
	jar -cf $(HOME_DIR)/$(JAR_FILE) $(SRC_DIR)/*.class

test:
	$(JAVA) -mx512M -classpath $(HOME_DIR)/$(JAR_FILE) crf.tagger.CRFTagger -modeldir $(MODEL_DIR) \
	-inputfile $(SAMPLES_DIR)/input.txt
	$(JAVA) -mx512M -classpath $(HOME_DIR)/$(JAR_FILE) crf.tagger.CRFTagger -modeldir $(MODEL_DIR) \
	-inputdir $(SAMPLES_DIR)/inputdir

clean:
	rm $(SRC_DIR)/*.class
	rm $(HOME_DIR)/$(JAR_FILE)
	rm $(SAMPLES_DIR)/*.pos
	rm $(SAMPLES_DIR)/inputdir/*.pos
