
class ParseExperiment:
  def __init__(self, original_test, prefix, gold_file, mrf_spec, model, penalty, training="wsj_gold_50"):
    self.original_test = original_test
    self.prefix = prefix
    self.gold_file = gold_file
    self.full_exp_prefix = prefix
    self._mrf_spec = mrf_spec
    self._model = model
    self._penalty = penalty
    self._model_pre = model.split('.')[0]
    self._training = training
  def test_file(self):
    return "$SCARAB_DATA/%s"%(self.original_test)

  def training_file(self):
    return "$SCARAB_DATA/%s"%(self._training)


  def parse_out(self):
    return "$SCARAB_TMP/%s_%s_parse"%(self.original_test, self._model_pre)

  def hypergraph_prefix(self):
    return "$SCARAB_TMP/%s_%s_hypergraph"%(self.original_test, self._model_pre)

  def constraint_mrf_prefix(self):
    return "$SCARAB_TMP/%s_parse_mrf"%(self.full_exp_prefix)

  def mrf_spec(self):
    return "$SCARAB_DATA/%s"%(self._mrf_spec)

  def model(self):
    return "$SCARAB_ROOT/third-party/mstparser/%s"%(self._model)


  def result_prefix(self):
    return "$SCARAB_RESULTS/%s"%(self.full_exp_prefix)

  def penalty(self):
    return self._penalty


# small_parse_experiment = ParseExperiment(
#   original_test= "english_test.conll",
#   prefix= "parse_first",
#   gold_file= "english_test.conll",
#   mrf_spec= "parse_constraints",
#   model = "small.model")

parse_experiment = ParseExperiment(
  original_test= "english_test.conll",
  prefix= "parse_bigger",
  gold_file= "english_test.conll",
  mrf_spec= "parse_constraints_200",
  model = "small.model",
  penalty = 0.1)

parse_experiment2 = ParseExperiment(
  original_test= "english_test.conll",
  prefix= "parse_bigger2",
  gold_file= "english_test.conll",
  mrf_spec= "parse_constraints_200",
  model = "small.model",
  penalty = 0.5)


parse_experiment_big = ParseExperiment(
  original_test= "english_test.conll",
  prefix= "parse_star",
  gold_file= "english_test.conll",
  mrf_spec= "parse_constraints_big",
  model = "small.model",
  penalty = 0.5)


parse_experiment_gentle = ParseExperiment(
  original_test= "english_test_bigger.conll",
  prefix= "parse_gentle",
  gold_file= "english_test_bigger.conll",
  mrf_spec= "parse_constraints_big",
  model = "small.model",
  penalty = 0.1)

parse_experiment_gentle2 = ParseExperiment(
  original_test= "english_test_bigger.conll",
  prefix= "parse_gentle2",
  gold_file= "english_test_bigger.conll",
  mrf_spec= "parse_constraints_big",
  model = "small.model",
  penalty = 0.5)

parse_experiment_wide = ParseExperiment(
  original_test= "english_test_bigger.conll",
  prefix= "parse_wide",
  gold_file= "english_test_bigger.conll",
  mrf_spec= "parse_constraints_size",
  model = "small.model",
  penalty = 0.5)


parse_experiment_gentle_full = ParseExperiment(
  original_test= "sec22_gold_ulab.conll",
  prefix= "parse_gentle_full",
  gold_file= "sec22_gold_ulab.conll",
  mrf_spec= "parse_constraints_big",
  model = "small.model",
  penalty = 0.1)


parse_experiment_gentle_size = ParseExperiment(
  original_test= "sec22_gold_ulab.conll",
  prefix= "parse_gentle_size",
  gold_file= "sec22_gold_ulab.conll",
  mrf_spec= "parse_constraints_size",
  model = "small.model",
  penalty = 0.1)

parse_experiment_gentle_mi_full = ParseExperiment(
  original_test= "sec22_gold_ulab.conll",
  prefix= "parse_gentle_mi_full",
  gold_file= "sec22_gold_ulab.conll",
  mrf_spec= "parse_constraints_mi",
  model = "small.model",
  penalty = 0.1)

parse_experiment_gentle_mi_small = ParseExperiment(
  original_test= "english_test.conll",
  prefix= "parse_gentle_mi_small",
  gold_file= "english_test.conll",
  mrf_spec= "parse_constraints_mi",
  model = "small.model",
  penalty = 0.1)


parse_experiment_gentle_mi_bigger = ParseExperiment(
  original_test= "english_test_bigger.conll",
  prefix= "parse_gentle_mi_bigger",
  gold_file= "english_test_bigger.conll",
  mrf_spec= "parse_constraints_mi",
  model = "small.model",
  penalty = 0.1)

parse_experiment_gentle_mi_bigger2 = ParseExperiment(
  original_test= "english_test_bigger.conll",
  prefix= "parse_gentle_mi_bigger2",
  gold_file= "english_test_bigger.conll",
  mrf_spec= "parse_constraints_mi",
  model = "small.model",
  penalty = 0.5)


parse_experiment_gentle_mihard_bigger = ParseExperiment(
  original_test= "english_test_bigger.conll",
  prefix= "parse_gentle_mihard_bigger",
  gold_file= "english_test_bigger.conll",
  mrf_spec= "parse_constraints_mi_hard",
  model = "small.model",
  penalty = 0.1)

parse_experiment_gentle_mi_asym_bigger = ParseExperiment(
  original_test= "english_test_bigger.conll",
  prefix= "parse_gentle_mi_asym_bigger",
  gold_file= "english_test_bigger.conll",
  mrf_spec= "parse_constraints_asym",
  model = "small.model",
  penalty = 0.1)

parse_experiment_gentle_mi_asym_full = ParseExperiment(
  original_test= "sec22_gold_ulab.conll",
  prefix= "parse_gentle_mi_asym_full",
  gold_file= "sec22_gold_ulab.conll",
  mrf_spec= "parse_constraints_asym",
  model = "small.model",
  penalty = 0.1)

parse_experiment_gentle_mi_asym2_full = ParseExperiment(
  original_test= "sec22_gold_ulab.conll",
  prefix= "parse_gentle_mi_asym2_full",
  gold_file= "sec22_gold_ulab.conll",
  mrf_spec= "parse_constraints_asym",
  model = "small.model",
  penalty = 0.3)

parse_experiment_gentle_mi_asymroi_full = ParseExperiment(
  original_test= "sec22_gold_ulab.conll",
  prefix= "parse_gentle_mi_asymroi_full",
  gold_file= "sec22_gold_ulab.conll",
  mrf_spec= "parse_constraints_asym_roi",
  model = "small.model",
  penalty = 0.3)


parse_experiment_gentle_mi_asymlow_full = ParseExperiment(
  original_test= "sec22_gold_ulab.conll",
  prefix= "parse_gentle_mi_asymlow_full",
  gold_file= "sec22_gold_ulab.conll",
  mrf_spec= "parse_constraints_asym_low",
  model = "small.model",
  penalty = 0.7)

parse_experiment_gentle_mi_asymlow_bigger = ParseExperiment(
  original_test= "english_test_bigger.conll",
  prefix= "parse_gentle_mi_asymlow_bigger",
  gold_file= "english_test_bigger.conll",
  mrf_spec= "parse_constraints_asym_low",
  model = "small.model",
  training = "wsj_gold_100",
  penalty = 0.7)


parse_experiment_strong_mi_asymlow_tiny = ParseExperiment(
  original_test= "english_test.conll",
  prefix= "parse_gentle_mi_asymlow_tiny",
  gold_file= "english_test.conll",
  mrf_spec= "parse_constraints_asym_low",
  model = "small.model",
  training = "wsj_gold_50",
  penalty = 0.7)

parse_experiment_strong_mi_asymlow_bigger_100 = ParseExperiment(
  original_test= "english_test_bigger.conll",
  prefix= "parse_strong_mi_asymlow_bigger_100",
  gold_file= "english_test_bigger.conll",
  mrf_spec= "parse_constraints_asym_low",
  model = "small100.model",
  penalty = 0.7)

parse_experiment_strong_mi_asymlow_bigger_500 = ParseExperiment(
  original_test= "english_test_bigger.conll",
  prefix= "parse_strong_mi_asymlow_bigger_500",
  gold_file= "english_test_bigger.conll",
  mrf_spec= "parse_constraints_asym_low",
  model = "small500.model",
  penalty = 0.7)

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
