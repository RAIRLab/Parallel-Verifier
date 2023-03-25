from .Justification import Justification

# Given a list of fstring, parses each, instantiates them into
# Formula objects, and returns a new list
def parse_list(fstrings):

  flist = []
  for f in fstrings:
    flist.append(parse_fstring(f))
  return flist



# Given a fstring, returns an instantiated Formula object
def parse_fstring(fstring):
  # Sanitize fstring
  # (Removes any superfluous spacing which will cause issues during parsing)
  fstring = " ".join(fstring.split())

  if(fstring.startswith("(Believes!")):
    if(fstring[10] == ' '):
      from .Belief import Belief
      return Belief.from_string(fstring, Justification(isgiven=True))
    else:
      from .LBelief import LBelief
      return LBelief.from_string(fstring, Justification(isgiven=True))

  elif(fstring.startswith("(Perceives!")):
    from .Perception import Perception
    return Perception.from_string(fstring, Justification(isgiven=True))

  elif(fstring.startswith("(Says!")):
    from .Says import Says
    return Says.from_string(fstring, Justification(isgiven=True))

  elif(fstring.startswith("(Knows!")):
    from .Knowledge import Knowledge
    return Knowledge.from_string(fstring, Justification(isgiven=True))

  elif(fstring.startswith("(Ought!")):
    from .Obligation import Obligation
    return Obligation.from_string(fstring, Justification(isgiven=True))

  # TODO Not implemented yet...
  #elif(fstring.startswith("(Common!")):
  #  return Common(fstring)

  #elif(fstring.startswith("(Intends!")):
  #  return Intends(fstring)

  #elif(fstring.startswith("(Desires!")):
  #  return Desires(fstring)

  elif(fstring.startswith("(Odds!")):
    from .Odds import Odds
    return Odds.from_string(fstring, Justification(isgiven=True))

  elif(fstring.startswith("(forall")):
    from .Universal import Universal
    return Universal.from_string(fstring, Justification(isgiven=True))

  elif(fstring.startswith("(exists")):
    from .Existential import Existential
    return Existential.from_string(fstring, Justification(isgiven=True))

  elif(fstring.startswith("(not")):
    from .Not import Not
    return Not.from_string(fstring, Justification(isgiven=True))

  elif(fstring.startswith("(and")):
    from .And import And
    return And.from_string(fstring, Justification(isgiven=True))

  elif(fstring.startswith("(or")):
    from .Or import Or
    return Or.from_string(fstring, Justification(isgiven=True))

  elif(fstring.startswith("(implies")):
    from .Implication import Implication
    return Implication.from_string(fstring, Justification(isgiven=True))

  elif(fstring.startswith("(iff")):
    from .BiConditional import BiConditional
    return BiConditional.from_string(fstring, Justification(isgiven=True))

  elif(fstring.startswith("(>")):
    from .Greater import Greater
    return Greater.from_string(fstring, Justification(isgiven=True))

  elif(fstring.startswith("(=")):
    from .Equal import Equal
    return Equal.from_string(fstring, Justification(isgiven=True))

  elif(fstring.startswith("?")):
    from .Variable import Variable
    return Variable(fstring)

  elif(fstring.startswith("(") and fstring[1].isupper()):
    from .Predicate import Predicate
    return Predicate.from_string(fstring, Justification(isgiven=True))

  elif(fstring.startswith("(") and fstring[1].islower()):
    from .Function import Function
    return Function.from_string(fstring, Justification(isgiven=True))

  elif(isint(fstring)):
    from .Integer import Integer
    return Integer.from_string(fstring, Justification(isgiven=True))

  elif(fstring[0].isalpha()):
    from .Atom import Atom
    return Atom.from_string(fstring, Justification(isgiven=True))

  else:
    print("ERROR: Improperly formatted fstring: " + fstring)
    exit(1)


# formula -- String
# Given a s-expression, returns a list of the top-level elements
# e.g. given (and (and a b) (implies (and c d) e))
# returns    ['and', '(and a b)', '(implies (and c d) e)']
def parse_sexpr(formula):
  out = []
  begin_atom = 1
  begin_search = 1
  depth = 0
  N = len(formula)

  while begin_search < N:
    end = find_next_of_interest(formula, begin_search)
    c = formula[end]
    begin_search = end + 1

    section = formula[begin_atom:end] # Potential sub s-expr to parse out
    n = len(section)

    # This case ensures that we catch non-parenthesized sub s-expr's
    # (e.g. propositional atoms, "and" in the example above
    #
    # If the current character is a space and depth == 0, we may want to
    # end the current sub-string and make it a sub s-expr. HOWEVER, when
    # there is extra space in the string, this will give each space its
    # own sub s-expr. The final two conditons prevent this.
    if(c == ' ' and depth == 0 and n > 0 and (not section == n*" ")):
        out.append(formula[begin_atom:end])
        begin_atom = end + 1

    elif(c == '('):
      depth += 1
      begin_search = end + 1

    elif(c == ')'):
      depth -= 1
      if(depth == 0):
        out.append(formula[begin_atom:end+1])
        begin_atom = end + 2
        begin_search = begin_atom

      # Done! (Hit last right paren)
      elif(depth == -1):
        out.append(formula[begin_atom:end])

  return out

# NOT INTENDED TO BE CALLED OUTSIDE THIS FILE
# Helper for parse_sexpr
# Finds the next character of interest in the formula
def find_next_of_interest(formula, begin):
  val = float("inf")

  idx = formula.find(' ', begin)
  if(not idx == -1 and idx < val): val = idx

  idx = formula.find('(', begin)
  if(not idx == -1 and idx < val): val = idx

  idx = formula.find(')', begin)
  if(not idx == -1 and idx < val): val = idx

  return val



def isint(val):
  try:
    int(val)
    return True
  except ValueError:
    return False

