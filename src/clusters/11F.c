#include "globals.h"
#include "bonds.h"
#include "tools.h"
#include "11F.h"
#include "13K.h"

void Clusters_Get11F_13K() {

    //!  An 11F cluster is the intersection of two 5A and two 6A clusters
    /*!
   *  Find 11F clusters
   *  An 11E is constructed from two 5A two 6A clusters where:
   *      - Each spindle of 5Ai is bonded to a spindle of 5Aj.
   *      - There is one common ring particle between the 5A clusters.
   *      - There is one bonded pair of ring particle between the 5A clusters.
   *      - Both 6As have one distinct spindle and one spindle which is the common ring particle of the 5As.
   *      - Each 6A has two bonded 5A spindles and two bonded 5A ring particles as its ring.
   *
   *  Cluster output: BOOOOOOBBBB
   *  Storage order: 5A_common particle, 6A_uncommon_spindle x 2, 5A_spindles x 4, 5A_ring_particles x 4)
   *
   */

    int first_5A_ring_pointer;

    int first_5A_id, second_5A_id, second_5A_pointer;
    int *first_5A_custer, *second_5A_cluster;
    int first_5A_ring_particle;

    int bonded_pairs[4];

    for(first_5A_id = 0; first_5A_id < nsp3c; first_5A_id++) {
        first_5A_custer = hcsp3c[first_5A_id];
        for (first_5A_ring_pointer = 0; first_5A_ring_pointer < 3; first_5A_ring_pointer++) {    // loop over only the rings of the 5A clusters
            first_5A_ring_particle = first_5A_custer[first_5A_ring_pointer];
            for (second_5A_pointer = 0; second_5A_pointer < nmem_sp3c[first_5A_ring_particle]; second_5A_pointer++) {
                second_5A_id = mem_sp3c[first_5A_ring_particle][second_5A_pointer];
                second_5A_cluster = hcsp3c[second_5A_id];
                if (second_5A_id > first_5A_id) {

                    // Check that the 5As have distinct spindles
                    if (do_5As_have_distinct_spindles(first_5A_custer, second_5A_cluster)) {

                        // Check that the sp5 spindles are bonded
                        if (Bonds_BondCheck(first_5A_custer[3], second_5A_cluster[3]) == 1 && Bonds_BondCheck(first_5A_custer[4], second_5A_cluster[4]) == 1) {
                            bonded_pairs[0] = first_5A_custer[3];
                            bonded_pairs[1] = second_5A_cluster[3];
                            bonded_pairs[2] = first_5A_custer[4];
                            bonded_pairs[3] = second_5A_cluster[4];
                            check_common_particle(first_5A_id, second_5A_id, bonded_pairs);
                        }

                        else if (Bonds_BondCheck(first_5A_custer[3], second_5A_cluster[4]) == 1 && Bonds_BondCheck(first_5A_custer[4], second_5A_cluster[3]) == 1) {
                            bonded_pairs[0] = first_5A_custer[3];
                            bonded_pairs[1] = second_5A_cluster[4];
                            bonded_pairs[2] = first_5A_custer[4];
                            bonded_pairs[3] = second_5A_cluster[3];
                            check_common_particle(first_5A_id, second_5A_id, bonded_pairs);
                        }
                    }
                }
            }
        }
    }
}

int do_5As_have_distinct_spindles(const int *first_5A, const int *second_5A) {
    return first_5A[3] != second_5A[3] && first_5A[3] != second_5A[4] && first_5A[4] != second_5A[4];
}

void check_common_particle(int first_5A_cluster_id, int second_5A_id, const int *bonded_pairs) {
    int common_particle;
    int num_bonded_pairs;
    int cluster_found;
    int bpi, bpj;
    int ep1, ep2;
    int bonded_6A_id;
    int *first_5A_cluster, *second_5A_cluster;

    first_5A_cluster = hcsp3c[first_5A_cluster_id];
    second_5A_cluster = hcsp3c[second_5A_id];

    common_particle = get_common_particle_between_5As(first_5A_cluster, second_5A_cluster);
    if (common_particle != -1) {
        // There must be only 1 particle common between the two 5As
        num_bonded_pairs = get_bonded_ring_particles(common_particle, first_5A_cluster, second_5A_cluster, &bpi, &bpj);
        // There must be exactly one paticle of first_5A bonded to exactly one of second_5A
        if (num_bonded_pairs == 1) {

            cluster_found = get_bonded_6As(common_particle, bpi, bpj, &ep1, &ep2, &bonded_6A_id, bonded_pairs);

            if (cluster_found) {
                write_11F(common_particle, ep1, ep2, first_5A_cluster, second_5A_cluster);

                if (do13K == 1) {
                    if (Clusters_Get13K(first_5A_cluster_id, second_5A_id, bonded_6A_id)) {
                        ++n13K;
                    }
                }
                ++n11F;
            }
        }
    }
}

int get_common_particle_between_5As(const int *first_5A, const int *second_5A) {
    // return id of common particle if only 1 common particle found, else return -1

    int i, j;
    int num_common_particles;
    int common_id = -1;

    num_common_particles = 0;
    for (i = 0; i < 3; i++) {
        for (j = 0; j < 3; j++) {
            if (first_5A[i] == second_5A[j]) {
                common_id = first_5A[i];
                num_common_particles++;
            }
        }
    }
    if(num_common_particles == 1) {
        return common_id;
    }
    else {
        return -1;
    }
}

int get_bonded_ring_particles(int cp, const int *first_5A, const int *second_5A, int *bpi, int *bpj) {
    int i, j, num_bonded_pairs;

    num_bonded_pairs = 0;
    for (i = 0; i < 3; i++) {
        for (j = 0; j < 3; j++) {
            if (first_5A[i] != cp && second_5A[j] != cp) {
                if (Bonds_BondCheck(first_5A[i], second_5A[j]) == 1) {
                    *bpi = first_5A[i];
                    *bpj = second_5A[j];
                    num_bonded_pairs++;
                }
            }
        }
    }
    return num_bonded_pairs;
}

int get_bonded_6As(int common_particle, int bpi, int bpj, int *ep1, int *ep2, int *bonded_6A_id, const int *bonded_pairs) {

    int first_6A_detected, second_6A_detected;
    int *first_6A;
    first_6A_detected = second_6A_detected = 0;

    // Loop over all 6As
    for (int potential_6A_pointer = 0; potential_6A_pointer < nsp4c; potential_6A_pointer++) {
        first_6A = hcsp4c[potential_6A_pointer];
        if (first_6A[4] == common_particle || first_6A[5] == common_particle) {

            if (first_6A_detected == 0) {
                first_6A_detected = get_first_6A(first_6A, bpi, bpj, common_particle, ep1, bonded_6A_id,
                                                 potential_6A_pointer, bonded_pairs);
            }
            if (second_6A_detected == 0) {
                second_6A_detected = get_second_6A(first_6A, bpi, bpj, common_particle, ep2, bonded_pairs);
            }
            if (first_6A_detected == 1 && second_6A_detected == 1) {
                return 1;
            }
        }
    }
    return 0;
}

int get_first_6A(const int *first_6A, int bpi, int bpj, int common_particle, int *ep1, int *bonded_6A_id,
                 int potential_6A_pointer, const int *bonded_pairs) {
    int i;
    int counter;

    for (i = 0; i < 4; i++) {
        counter = first_6A[i] == bonded_pairs[0] || first_6A[i] == bonded_pairs[1] || first_6A[i] == bpi || first_6A[i] == bpj;
        if (counter == 0) break;
    }
    if (i == 4) {
        if (first_6A[4] == common_particle) *ep1 = first_6A[5];
        else *ep1 = first_6A[4];
        *bonded_6A_id = potential_6A_pointer;
        return 1;
    }
    return 0;
}

int get_second_6A(const int *first_6A, int bpi, int bpj, int common_particle, int *ep2, const int *bonded_pairs) {
    int i;
    int counter;

    for (i = 0; i < 4; i++) {
        counter = first_6A[i] == bonded_pairs[2] || first_6A[i] == bonded_pairs[3] || first_6A[i] == bpi || first_6A[i] == bpj;
        if (counter == 0) break;
    }
    if (i == 4) {
        if (first_6A[4] == common_particle) *ep2 = first_6A[5];
        else *ep2 = first_6A[4];
        return 1;
    }
    return 0;
}


void write_11F(int common_particle, int ep1, int ep2, const int *first_5A, const int *second_5A) {
    int clusSize = 11;

    if (n11F == m11F) {
        hc11F = resize_2D_int(hc11F, m11F, m11F + incrStatic, clusSize, -1);
        m11F = m11F + incrStatic;
    }

    hc11F[n11F][0] = common_particle;
    hc11F[n11F][1] = ep1;
    hc11F[n11F][2] = ep2;
    hc11F[n11F][3] = first_5A[3];
    hc11F[n11F][4] = first_5A[4];
    hc11F[n11F][5] = second_5A[3];
    hc11F[n11F][6] = second_5A[4];

    int j = 7;
    for (int i = 0; i < 3; ++i) {
        if (first_5A[i] != common_particle) {
            hc11F[n11F][j] = first_5A[i];
            j++;
        }
        if (second_5A[i] != common_particle) {
            hc11F[n11F][j] = second_5A[i];
            j++;
        }
    }

    if (s11F[hc11F[n11F][0]] == 'C') s11F[hc11F[n11F][0]] = 'B';
    for(int i = 1; i < 7; i++) {
        s11F[hc11F[n11F][i]] = 'O';
    }
    for(int i = 7; i < 11; i++) {
        if (s11F[hc11F[n11F][i]] == 'C') s11F[hc11F[n11F][i]] = 'B';
    }
}