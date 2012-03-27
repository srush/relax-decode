class Experiment:
  def __init__(self, desc):
    self.desc = desc
    self.model_type = desc.get("model_type",'nbayes')
    self.prefix = desc["prefix"]
    self.unk_exp_prefix =  desc["prefix"] + "_" + str(desc['unk_thres']) 
    self.full_exp_prefix = self.unk_exp_prefix +  "_" + str(int(10*desc['penalty'])) + "_" + self.model_type
    self.train = desc["train"]
    self.unk_thres = desc['unk_thres']
    #self.num_sentences = desc["num_sentences"]
    #self.num_cons = desc.get("num_constraints", 0)
    self._penalty = desc["penalty"]
    self.original_test = desc["original_test"]
    self._gold_file = desc["gold_file"]
    self._tagger = desc["tagger"]
    self._num_sentences = desc.get("num_sentences", 10)
    self._add_sent = desc.get("add_sents", False)  
    self.language = desc.get("language", "english")

  def constraint_type(self):
    return self.model_type

  def add_sent(self):
    return self._add_sent
  
  def original_test_file(self):
    return "$SCARAB_DATA/" + self.original_test

  def full_test_file(self):
    return "$SCARAB_DATA/" + self.prefix

  def full_test_file_desc(self):
    return "$SCARAB_DATA/" + self.prefix + "_desc"

  def build_constraint_file(self):
    return "$SCARAB_DATA/" + self.prefix + "_build_desc"

  def gold_file(self):
    return "$SCARAB_DATA/" + self._gold_file

  def train_file(self):
    return "$SCARAB_DATA/" + self.train 

  def unknown_words(self):
    return"$SCARAB_DATA/" + self.unk_exp_prefix

  def unknown_prefix(self):
    return "$SCARAB_DATA/" + self.unk_exp_prefix


  def tagger_model(self):
    return "$SCARAB_ROOT/third-party/stanford-postagger-2009-12-24/models/" + self._tagger

  def constraint_old_prefix(self):
    return "$SCARAB_DATA/" + self.unk_exp_prefix

  def mrf_link_prefix(self):
    return "$SCARAB_DATA/" + self.full_exp_prefix + "_link"

  def mrf_link_prefix_desc(self):
    return "$SCARAB_DATA/" + self.full_exp_prefix + "_link_desc"

  def brown_clusters(self):
    return "$SCARAB_DATA/paths"


  def constraint_mrf_prefix(self):
    return "$SCARAB_TMP/%s_mrf"%(self.full_exp_prefix)

  def result_prefix(self):
    return "$SCARAB_RESULTS/%s"%(self.full_exp_prefix)

  def lattice_prefix(self):
    return "$SCARAB_TMP/%s_lattice"%(self.unk_exp_prefix)

  def temporary_tags(self):
    return "$SCARAB_TMP/%s_tag"%(self.unk_exp_prefix)


  def temporary_margs(self):
    return "$SCARAB_TMP/%s_marginals"%(self.unk_exp_prefix)
  
  def num_sent(self):
    return self._num_sentences

  def num_constraints(self):
    return self.num_cons

  def penalty(self):
    return self._penalty
little_experiment = Experiment({
  "original_test": "english_test_pos",
  "prefix": "eng_little",
  "gold_file": "english_test_gold",
  "tagger": "left3words-wsj-50sent.tagger", 
  "unk_thres" : 5,
  "train" : "wsj_gold_50",
  "penalty" : 3.0,
  "add_sents": False,
  "num_sentences": 16
  })

little_experiment_add = Experiment({
  "original_test": "english_test_pos",
  "prefix": "eng_little_add",
  "gold_file": "english_test_gold",
  "tagger": "left3words-wsj-50sent.tagger", 
  "unk_thres" : 5,
  "train" : "wsj_gold_50",
  "penalty" : 5.0,
  "add_sents": True,
  "num_sentences": 16
  })

little_experiment_add2 = Experiment({
  "original_test": "english_test_pos",
  "prefix": "eng_little_add2",
  "gold_file": "english_test_gold",
  "tagger": "left3words-wsj-50sent.tagger", 
  "unk_thres" : 5,
  "train" : "wsj_gold_50",
  "penalty" : 5.0,
  "add_sents": True,
  "num_sentences": 16
  })


little_experiment_add2_small = Experiment({
  "original_test": "english_test_pos",
  "prefix": "eng_little_add2",
  "gold_file": "english_test_gold",
  "tagger": "left3words-wsj-50sent.tagger", 
  "unk_thres" : 5,
  "train" : "wsj_gold_50",
  "penalty" : 1.0,
  "add_sents": True,
  "num_sentences": 16
  })

little_experiment_add2_mid = Experiment({
  "original_test": "english_test_pos",
  "prefix": "eng_little_add2",
  "gold_file": "english_test_gold",
  "tagger": "left3words-wsj-50sent.tagger", 
  "unk_thres" : 5,
  "train" : "wsj_gold_50",
  "penalty" : 3.0,
  "add_sents": True,
  "num_sentences": 16
  })


small_experiment_add2_mid = Experiment({
  "original_test": "english_test_bigger_pos",
  "prefix": "eng_small_add",
  "gold_file": "english_test_small2_gold",
  "tagger": "left3words-wsj-50sent.tagger", 
  "unk_thres" : 5,
  "train" : "wsj_gold_50",
  "penalty" : 3.0,
  "add_sents": True,
  "num_sentences": 10
  })

small_experiment_add2_heavy = Experiment({
  "original_test": "english_test_bigger_pos",
  "prefix": "eng_small_add",
  "gold_file": "english_test_small2_gold",
  "tagger": "left3words-wsj-50sent.tagger", 
  "unk_thres" : 5,
  "train" : "wsj_gold_50",
  "penalty" : 5.0,
  "add_sents": True,
  "num_sentences": 10
  })


bigger_experiment = Experiment({
  "original_test": "english_test_bigger_pos",
  "prefix": "eng_bigger",
  "gold_file": "english_test_bigger_gold",
  "tagger": "left3words-wsj-50sent.tagger", 
  "unk_thres" : 5,
  "train" : "wsj_gold_50",
  "penalty" : 5.0,
  "add_sents": False,
  "num_sentences": 180
  })

bigger_experiment2 = Experiment({
  "original_test": "english_test_bigger_pos",
  "prefix": "eng_bigger2",
  "gold_file": "english_test_bigger_gold",
  "tagger": "left3words-wsj-50sent.tagger", 
  "unk_thres" : 5,
  "train" : "wsj_gold_50",
  "penalty" : 5.0,
  "add_sents": False,
  "num_sentences": 180
  })

bigger_experiment_add = Experiment({
  "original_test": "english_test_bigger_pos",
  "prefix": "eng_bigger_add",
  "gold_file": "english_test_bigger_gold",
  "tagger": "left3words-wsj-50sent.tagger", 
  "unk_thres" : 5,
  "train" : "wsj_gold_50",
  "penalty" : 5.0,
  "add_sents": True,
  "num_sentences": 180
  })

dev_experiment_add = Experiment({
  "original_test": "sec22_pos",
  "prefix": "eng_dev",
  "gold_file": "sec22_pos_gold",
  "tagger": "left3words-wsj-50sent.tagger", 
  "unk_thres" : 5,
  "train" : "wsj_gold_50",
  "penalty" : 5.0,
  "add_sents": True,
  "num_sentences": 1376
  })

def dev_experiment(num):
  return Experiment({
  "original_test": "dev_data/sec22_pos_%d"%num,
  "prefix": "eng_dev_%03d"%num,
  "gold_file": "dev_data/sec22_pos_gold_%d"%num,
  "tagger": "left3words-wsj-50sent.tagger", 
  "unk_thres" : 5,
  "train" : "wsj_gold_50",
  "penalty" : 3.0,
  "add_sents": True,
  "num_sentences": 5
  })


def dev_experiment_100(num):
  return Experiment({
  "original_test": "dev_data/sec22_pos_%d"%num,
  "prefix": "eng_dev_100_%03d"%num,
  "gold_file": "dev_data/sec22_pos_gold_%d"%num,
  "tagger": "english_100.model", 
  "unk_thres" : 5,
  "train" : "wsj_gold_100",
  "penalty" : 3.0,
  "add_sents": True,
  "num_sentences": 5
  })


def dev_experiment_100_lower(num):
  return Experiment({
  "original_test": "dev_data/sec22_pos_%d"%num,
  "prefix": "eng_dev_100_%03d"%num,
  "gold_file": "dev_data/sec22_pos_gold_%d"%num,
  "tagger": "english_100.model", 
  "unk_thres" : 5,
  "train" : "wsj_gold_100",
  "penalty" : 1.0,
  "add_sents": True,
  "num_sentences": 5
  })


german_exp = Experiment({
  "original_test": "german_dev",
  "prefix": "german_first",
  "gold_file": "german_dev_gold",
  "tagger": "german_50.model", 
  "unk_thres" : 5,
  "train" : "german_gold_50",
  "penalty" : 3.0,
  "add_sents": True,
  "num_sentences": 25,
  "language" : "german"
  })

def test_experiment(num, size=50):
  return Experiment({
  "original_test": "pos_test_data/sec23_%d"%num,
  "prefix": "eng_test_%s_%03d"%(size,num),
  "gold_file": "pos_test_data/sec23_%d"%num,
  #"tagger": "left3words-wsj-50sent.tagger",
  "tagger": "english_%s.model"%size, 
  "unk_thres" : 5,
  "train" : "wsj_gold_%s"%size,
  "penalty" : 3.0,
  "add_sents": True,
  "num_sentences": 5
  })

small_experiment = Experiment({
  "original_test": "brown_simple_1",
  "prefix": "brown_small_extra",
  "gold_file": "brown_small_extra_gold",
  "tagger": "left3words-wsj-50sent.tagger", 
  "unk_thres" : 5,
  "train" : "wsj_gold_50",
  "penalty" : 5.0
  })

small_experiment2 = Experiment({
  "original_test": "brown_simple_1",
  "prefix": "brown_base",
  "gold_file": "brown_small_extra_test",
  "tagger": "left3words-wsj-0-18.tagger", 
  "unk_thres" : 5,
  "train" : "wsj_gold_dependency",
  "penalty" : 5.0
  })

physics_experiment = Experiment({
  "original_test": "physics_corpus/physics_sentences_tokenized",
  "num_sentences": 25,
  "prefix": "physics_exper",
  "gold_file": "physics_corpus/physics_sent",
  "tagger": "left3words-wsj-0-18.tagger", 
  "unk_thres" : 5,
  "train" : "wsj_gold_dependency",
  "penalty" : 5.0
  })

religious_experiment = Experiment({
  "original_test": "religious/brown_religious_texts_stripped",
  "num_sentences": 25,
  "prefix": "religious_exper",
  "gold_file": "religious/brown_religious_texts_gold",
  "tagger": "left3words-wsj-0-18.tagger", 
  "unk_thres" : 5,
  "train" : "wsj_gold_dependency",
  "penalty" : 5.0
  })

qtb_experiment = Experiment({
  "original_test": "question_bank_new_dependency_simple_100",
  "num_sentences": 100,
  "prefix": "qtb_exper",
  "gold_file": "question_bank_new_dependency_simple_100",
  "tagger": "left3words-wsj-0-18.tagger", 
  "unk_thres" : 5,
  "train" : "wsj_gold_dependency",
  "penalty" : 5.0
  })



# test_expermient = Experiment({
#     "prefix": "brown_extra20", 
#     #"exp_prefix": "brown_simple_1_100", 
#     "model_type": "nbayes",
#     "num_sentences" : 382,
#     "num_constraints" : 157,
    
#     "unk_thres" : 200,
#     "train" : "wsj_gold_dependency",
#     "penalty" : 2.0
#     })
