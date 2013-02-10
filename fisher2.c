#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h>
#include <math.h>

// Fisher 2x2 and 2x3 exact test command line utility
// Copyright (C) 2013  Christopher Chang  chrchang@alumni.caltech.edu

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.


#define SMALLISH_EPSILON 0.00000000003
#define SMALL_EPSILON 0.0000000000001
#define DOUBLE_PREC_LIMIT 0.0000000000000001

double fisher22(uint32_t m11, uint32_t m12, uint32_t m21, uint32_t m22) {
  // Basic 2x2 Fisher exact test p-value calculation.
  double tprob = 1 - SMALL_EPSILON;
  double cur_prob = 1 - SMALL_EPSILON;
  double cprob = 0;
  double tail_stop;
  uint32_t uii;
  int64_t cur11; // signed integers convert to doubles more easily
  int64_t cur12;
  int64_t cur21;
  int64_t cur22;
  // Ensure we are left of the distribution center, m11 <= m22, and m12 <= m21.
  if (m12 > m21) {
    uii = m12;
    m12 = m21;
    m21 = uii;
  }
  if (m11 > m22) {
    uii = m11;
    m11 = m22;
    m22 = uii;
  }
  if ((((int64_t)m11) * m22) > (((int64_t)m12) * m21)) {
    uii = m11;
    m11 = m12;
    m12 = uii;
    uii = m21;
    m21 = m22;
    m22 = uii;
  }
  cur11 = m11;
  cur12 = m12;
  cur21 = m21;
  cur22 = m22;
  while (cur12) {
    cur_prob *= ((double)((cur12--) * (cur21--))) / ((double)((++cur11) * (++cur22)));
    if (cur_prob == INFINITY) {
      return 0;
    }
    if (cur_prob < 1) {
      tprob += cur_prob;
      break;
    }
    cprob += cur_prob;
  }
  if (cprob == 0) {
    return 1;
  }
  if (cur12) {
    tail_stop = tprob * DOUBLE_PREC_LIMIT;
    do {
      cur_prob *= ((double)((cur12--) * (cur21--))) / ((double)((++cur11) * (++cur22)));
      if (cur_prob < tail_stop) {
	break;
      }
      tprob += cur_prob;
    } while (cur12);
  }
  if (m11) {
    cur11 = m11;
    cur12 = m12;
    cur21 = m21;
    cur22 = m22;
    cur_prob = 1;
    tail_stop = tprob * DOUBLE_PREC_LIMIT;
    do {
      cur_prob *= ((double)((cur11--) * (cur22--))) / ((double)((++cur12) * (++cur21)));
      if (cur_prob < tail_stop) {
        return tprob / (cprob + tprob);
      }
      tprob += cur_prob;
    } while (cur11);
  }
  return tprob / (cprob + tprob);
}

int32_t fisher23_tailsum(double tail_stop, double* base_probp, double* saved12p, double* saved13p, double* saved22p, double* saved23p, double *totalp, uint32_t right_side) {
  double total = 0;
  double cur_prob = *base_probp;
  double tmp12 = *saved12p;
  double tmp13 = *saved13p;
  double tmp22 = *saved22p;
  double tmp23 = *saved23p;
  double prev_prob;
  double tmps12;
  double tmps13;
  double tmps22;
  double tmps23;
  // identify beginning of tail
  if (right_side) {
    if (cur_prob > 1) {
      if ((tmp13 < 0.5) || (tmp22 < 0.5)) {
	*totalp = 0;
	return 0;
      }
      do {
	tmp12 += 1;
	tmp23 += 1;
	cur_prob *= (tmp13 * tmp22) / (tmp12 * tmp23);
	tmp13 -= 1;
	tmp22 -= 1;
      } while (cur_prob > 1);
      *saved12p = tmp12;
      *saved13p = tmp13;
      *saved22p = tmp22;
      *saved23p = tmp23;
      *base_probp = cur_prob;
    } else {
      tmps12 = tmp12;
      tmps13 = tmp13;
      tmps22 = tmp22;
      tmps23 = tmp23;
      while (1) {
	prev_prob = cur_prob;
	tmp13 += 1;
	tmp22 += 1;
	cur_prob = (prev_prob * tmp12 * tmp23) / (tmp13 * tmp22);
	if (cur_prob < prev_prob) {
	  return 1;
	}
	tmp12 -= 1;
	tmp23 -= 1;
	if (cur_prob > 1 - SMALL_EPSILON) {
	  break;
	}
	total += cur_prob;
      }
      prev_prob = cur_prob;
      cur_prob = *base_probp;
      *base_probp = prev_prob;
      *saved12p = tmp12;
      *saved13p = tmp13;
      *saved22p = tmp22;
      *saved23p = tmp23;
      tmp12 = tmps12;
      tmp13 = tmps13;
      tmp22 = tmps22;
      tmp23 = tmps23;
    }
  } else {
    if (cur_prob > 1) {
      if ((tmp12 < 0.5) || (tmp23 < 0.5)) {
	*totalp = 0;
	return 0;
      }
      do {
	tmp13 += 1;
	tmp22 += 1;
	cur_prob *= (tmp12 * tmp23) / (tmp13 * tmp22);
	tmp12 -= 1;
	tmp23 -= 1;
      } while (cur_prob > 1);
      *saved12p = tmp12;
      *saved13p = tmp13;
      *saved22p = tmp22;
      *saved23p = tmp23;
      *base_probp = cur_prob;
    } else {
      tmps12 = tmp12;
      tmps13 = tmp13;
      tmps22 = tmp22;
      tmps23 = tmp23;
      while (1) {
	prev_prob = cur_prob;
	tmp12 += 1;
	tmp23 += 1;
	cur_prob = (prev_prob * tmp13 * tmp22) / (tmp12 * tmp23);
	if (cur_prob < prev_prob) {
	  return 1;
	}
	tmp13 -= 1;
	tmp22 -= 1;
	if (cur_prob > 1) {
	  break;
	}
	total += cur_prob;
      }
      prev_prob = cur_prob;
      cur_prob = *base_probp;
      *base_probp = prev_prob;
      *saved12p = tmp12;
      *saved13p = tmp13;
      *saved22p = tmp22;
      *saved23p = tmp23;
      tmp12 = tmps12;
      tmp13 = tmps13;
      tmp22 = tmps22;
      tmp23 = tmps23;
    }
  }
  // sum tail to floating point precision limit
  if (right_side) {
    while (cur_prob > tail_stop) {
      total += cur_prob;
      tmp12 += 1;
      tmp23 += 1;
      cur_prob *= (tmp13 * tmp22) / (tmp12 * tmp23);
      tmp13 -= 1;
      tmp22 -= 1;
    }
  } else {
    while (cur_prob > tail_stop) {
      total += cur_prob;
      tmp13 += 1;
      tmp22 += 1;
      cur_prob *= (tmp12 * tmp23) / (tmp13 * tmp22);
      tmp12 -= 1;
      tmp23 -= 1;
    }
  }
  *totalp = total;
  return 0;
}

double fisher23(uint32_t m11, uint32_t m12, uint32_t m13, uint32_t m21, uint32_t m22, uint32_t m23) {
  // 2x3 Fisher-Freeman-Halton exact test p-value calculation.
  // The number of tables involved here is still small enough that the network
  // algorithm (and the improved variants thereof that I've seen) are
  // suboptimal; a 2-dimensional version of the SNPHWE2 strategy has higher
  // performance.
  // 2x4, 2x5, and 3x3 should also be practical with this method, but beyond
  // that I doubt it's worth the trouble.
  double cur_prob = 1 - SMALLISH_EPSILON;
  double tprob = cur_prob;
  double cprob = 0;
  uint32_t dir = 0; // 0 = forwards, 1 = backwards
  double tail_stop = 0;
  double dyy = 0;
  double base_probl;
  double base_probr;
  double orig_base_probl;
  double orig_base_probr;
  double orig_row_prob;
  double row_prob;
  uint32_t uii;
  uint32_t ujj;
  uint32_t ukk;
  double cur11;
  double cur21;
  double savedl12;
  double savedl13;
  double savedl22;
  double savedl23;
  double savedr12;
  double savedr13;
  double savedr22;
  double savedr23;
  double orig_savedl12;
  double orig_savedl13;
  double orig_savedl22;
  double orig_savedl23;
  double orig_savedr12;
  double orig_savedr13;
  double orig_savedr22;
  double orig_savedr23;
  double tmp12;
  double tmp13;
  double tmp22;
  double tmp23;
  double dxx;
  // Ensure m11 + m21 <= m12 + m22 <= m13 + m23.
  uii = m11 + m21;
  ujj = m12 + m22;
  if (uii > ujj) {
    ukk = m11;
    m11 = m12;
    m12 = ukk;
    ukk = m21;
    m21 = m22;
    m22 = ukk;
    ukk = uii;
    uii = ujj;
    ujj = ukk;
  }
  ukk = m13 + m23;
  if (ujj > ukk) {
    ujj = ukk;
    ukk = m12;
    m12 = m13;
    m13 = ukk;
    ukk = m22;
    m22 = m23;
    m23 = ukk;
  }
  if (uii > ujj) {
    ukk = m11;
    m11 = m12;
    m12 = ukk;
    ukk = m21;
    m21 = m22;
    m22 = ukk;
  }
  // Ensure majority of probability mass is in front of m11.
  if ((((uint64_t)m11) * (m22 + m23)) > (((uint64_t)m21) * (m12 + m13))) {
    ukk = m11;
    m11 = m21;
    m21 = ukk;
    ukk = m12;
    m12 = m22;
    m22 = ukk;
    ukk = m13;
    m13 = m23;
    m23 = ukk;
  }
  if ((((uint64_t)m12) * m23) > (((uint64_t)m13) * m22)) {
    base_probr = cur_prob;
    savedr12 = m12;
    savedr13 = m13;
    savedr22 = m22;
    savedr23 = m23;
    tmp12 = savedr12;
    tmp13 = savedr13;
    tmp22 = savedr22;
    tmp23 = savedr23;
    if ((tmp12 > 0.5) && (tmp23 > 0.5)) {
      while (1) {
	tmp13 += 1;
	tmp22 += 1;
	cur_prob *= (tmp12 * tmp23) / (tmp13 * tmp22);
	tmp12 -= 1;
	tmp23 -= 1;
	if (cur_prob <= 1) {
	  break;
	}
	cprob += cur_prob;
	if (cprob == INFINITY) {
	  return 0;
	}
      }
    }
    savedl12 = tmp12;
    savedl13 = tmp13;
    savedl22 = tmp22;
    savedl23 = tmp23;
    base_probl = cur_prob;
    tprob += cur_prob;
    tail_stop = tprob * DOUBLE_PREC_LIMIT;
    while (cur_prob > tail_stop) {
      tmp13 += 1;
      tmp22 += 1;
      cur_prob *= (tmp12 * tmp23) / (tmp13 * tmp22);
      tmp12 -= 1;
      tmp23 -= 1;
      tprob += cur_prob;
    }
    tmp12 = savedr12;
    tmp13 = savedr13;
    tmp22 = savedr22;
    tmp23 = savedr23;
    tail_stop = tprob * DOUBLE_PREC_LIMIT;
    cur_prob = base_probr;
    do {
      tmp12 += 1;
      tmp23 += 1;
      cur_prob *= (tmp13 * tmp22) / (tmp12 * tmp23);
      tmp13 -= 1;
      tmp22 -= 1;
      tprob += cur_prob;
    } while (cur_prob > tail_stop);
  } else {
    base_probl = cur_prob;
    savedl12 = m12;
    savedl13 = m13;
    savedl22 = m22;
    savedl23 = m23;
    tmp12 = savedl12;
    tmp13 = savedl13;
    tmp22 = savedl22;
    tmp23 = savedl23;
    if ((tmp13 > 0.5) && (tmp22 > 0.5)) {
      while (1) {
	tmp12 += 1;
	tmp23 += 1;
	cur_prob *= (tmp13 * tmp22) / (tmp12 * tmp23);
	tmp13 -= 1;
	tmp22 -= 1;
	if (cur_prob <= 1) {
	  break;
	}
	cprob += cur_prob;
	if (cprob == INFINITY) {
	  return 0;
	}
      }
    }
    savedr12 = tmp12;
    savedr13 = tmp13;
    savedr22 = tmp22;
    savedr23 = tmp23;
    base_probr = cur_prob;
    tprob += cur_prob;
    tail_stop = tprob * DOUBLE_PREC_LIMIT;
    while (cur_prob > tail_stop) {
      tmp12 += 1;
      tmp23 += 1;
      cur_prob *= (tmp13 * tmp22) / (tmp12 * tmp23);
      tmp13 -= 1;
      tmp22 -= 1;
      tprob += cur_prob;
    }
    tmp12 = savedl12;
    tmp13 = savedl13;
    tmp22 = savedl22;
    tmp23 = savedl23;
    tail_stop = tprob * DOUBLE_PREC_LIMIT;
    cur_prob = base_probl;
    do {
      tmp13 += 1;
      tmp22 += 1;
      cur_prob *= (tmp12 * tmp23) / (tmp13 * tmp22);
      tmp12 -= 1;
      tmp23 -= 1;
      tprob += cur_prob;
    } while (cur_prob > tail_stop);
  }
  row_prob = tprob + cprob;
  orig_base_probl = base_probl;
  orig_base_probr = base_probr;
  orig_row_prob = row_prob;
  orig_savedl12 = savedl12;
  orig_savedl13 = savedl13;
  orig_savedl22 = savedl22;
  orig_savedl23 = savedl23;
  orig_savedr12 = savedr12;
  orig_savedr13 = savedr13;
  orig_savedr22 = savedr22;
  orig_savedr23 = savedr23;
  ukk = 0;
  for (; dir < 2; dir++) {
    cur11 = m11;
    cur21 = m21;
    if (dir) {
      base_probl = orig_base_probl;
      base_probr = orig_base_probr;
      row_prob = orig_row_prob;
      savedl12 = orig_savedl12;
      savedl13 = orig_savedl13;
      savedl22 = orig_savedl22;
      savedl23 = orig_savedl23;
      savedr12 = orig_savedr12;
      savedr13 = orig_savedr13;
      savedr22 = orig_savedr22;
      savedr23 = orig_savedr23;
    }
    while (1) {
      if (dir) {
	if ((cur11 < 0.5) || (savedl22 + savedl23 < 0.5)) {
	  ukk = 1;
	  break;
	}
	cur21 += 1;
	if (savedl23 > savedl22) {
	  savedl13 += 1;
	  row_prob *= (cur11 * (savedl22 + savedl23)) / (cur21 * (savedl12 + savedl13));
	  base_probl *= (cur11 * savedl23) / (cur21 * savedl13);
	  savedl23 -= 1;
	} else {
	  savedl12 += 1;
	  row_prob *= (cur11 * (savedl22 + savedl23)) / (cur21 * (savedl12 + savedl13));
	  base_probl *= (cur11 * savedl22) / (cur21 * savedl12);
	  savedl22 -= 1;
	}
	cur11 -= 1;
      } else {
	if ((cur21 < 0.5) || (savedl12 + savedl13 < 0.5)) {
	  ukk = 1;
	  break;
	}
	cur11 += 1;
	if (savedl13 > savedl12) {
	  savedl23 += 1;
	  row_prob *= (cur21 * (savedl12 + savedl13)) / (cur11 * (savedl22 + savedl23));
	  base_probl *= (cur21 * savedl13) / (cur11 * savedl23);
	  savedl13 -= 1;
	} else {
	  savedl22 += 1;
	  row_prob *= (cur21 * (savedl12 + savedl13)) / (cur11 * (savedl22 + savedl23));
	  base_probl *= (cur21 * savedl12) / (cur11 * savedl22);
	  savedl12 -= 1;
	}
	cur21 -= 1;
      }
      tail_stop = tprob * DOUBLE_PREC_LIMIT;
      if (fisher23_tailsum(tail_stop, &base_probl, &savedl12, &savedl13, &savedl22, &savedl23, &dxx, 0)) {
	break;
      }
      tprob += dxx;
      if (dir) {
	if (savedr23 > savedr22) {
	  savedr13 += 1;
	  base_probr *= ((cur11 + 1) * savedr23) / (cur21 * savedr13);
	  savedr23 -= 1;
	} else {
	  savedr12 += 1;
	  base_probr *= ((cur11 + 1) * savedr22) / (cur21 * savedr12);
	  savedr22 -= 1;
	}
      } else {
	if (savedr13 > savedr12) {
	  savedr23 += 1;
	  base_probr *= ((cur21 + 1) * savedr13) / (cur11 * savedr23);
	  savedr13 -= 1;
	} else {
	  savedr22 += 1;
	  base_probr *= ((cur21 + 1) * savedr12) / (cur11 * savedr22);
	  savedr12 -= 1;
	}
      }
      tail_stop = tprob * DOUBLE_PREC_LIMIT;
      fisher23_tailsum(tail_stop, &base_probr, &savedr12, &savedr13, &savedr22, &savedr23, &dyy, 1);
      tprob += dyy;
      cprob += row_prob - dxx - dyy;
      if (cprob == INFINITY) {
	return 0;
      }
    }
    if (ukk) {
      continue;
    }
    tail_stop = tprob * DOUBLE_PREC_LIMIT;
    savedl12 += savedl13;
    savedl22 += savedl23;
    if (dir) {
      while (row_prob > tail_stop) {
	tprob += row_prob;
	cur21 += 1;
	savedl12 += 1;
	row_prob *= (cur11 * savedl22) / (cur21 * savedl12);
	cur11 -= 1;
	savedl22 -= 1;
      }
    } else {
      while (row_prob > tail_stop) {
	tprob += row_prob;
	cur11 += 1;
	savedl22 += 1;
	row_prob *= (cur21 * savedl12) / (cur11 * savedl22);
	cur21 -= 1;
	savedl12 -= 1;
      }
    }
  }
  return tprob / (tprob + cprob);
}

int main(int argc, char** argv) {
  if (argc == 5) {
    printf("p-value: %g\n", fisher22(atoi(argv[1]), atoi(argv[2]), atoi(argv[3]), atoi(argv[4])));
  } else if (argc == 7) {
    printf("p-value: %g\n", fisher23(atoi(argv[1]), atoi(argv[3]), atoi(argv[5]), atoi(argv[2]), atoi(argv[4]), atoi(argv[6])));
  } else {
    printf(
"Fisher 2x2 and 2x3 exact test    https://www.cog-genomics.org/wdist\n"
"(C) 2013 Christopher Chang, GNU General Public License version 3\n\n"
"Usage: fisher [m11] [m12] [m21] [m22]\n"
"       fisher [m11] [m12] [m21] [m22] [m31] [m32]\n"
	   );
    return 1;
  }
  return 0;
}