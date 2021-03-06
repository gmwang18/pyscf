/*
 * Author: Qiming Sun <osirpt.sun@gmail.com>
 */

#include <stdlib.h>
#include <assert.h>
#include <complex.h>
//#include <omp.h>
#include "config.h"
#include "cint.h"
#include "vhf/fblas.h"
#include "vhf/nr_direct.h"
#include "np_helper/np_helper.h"
#include "ao2mo/r_ao2mo.h"

void zhemm_(const char*, const char*,
            const int*, const int*,
            const double complex*, const double complex*, const int*,
            const double complex*, const int*,
            const double complex*, double complex*, const int*);

void RIfill_r_s1_auxe2(int (*intor)(), double complex *eri,
                       int ish, int jsh, int bastart, int auxstart, int auxcount,
                       CINTOpt *cintopt, struct _VHFEnvs *envs)
{
        const int nao = envs->nao;
        const int *ao_loc = envs->ao_loc;
        const int di = ao_loc[ish+1] - ao_loc[ish];
        const int dj = ao_loc[jsh+1] - ao_loc[jsh];
        const int dij = di * dj;
        const int nbasnaux = auxstart + auxcount;
        const int naoaux = ao_loc[nbasnaux] - ao_loc[auxstart];
        double complex *eribuf = malloc(sizeof(double complex)*di*dj*naoaux);

        int ksh, dk;
        int i, j, k, i0, j0, k0;
        int shls[3];
        size_t ij0;
        double complex *peri, *pbuf;

        shls[0] = ish;
        shls[1] = jsh;
        for (ksh = auxstart; ksh < nbasnaux; ksh++) {
                shls[2] = ksh;
                k0 = ao_loc[ksh] - ao_loc[auxstart];;
                dk = ao_loc[ksh+1] - ao_loc[ksh];
                i0 = ao_loc[ish] - ao_loc[bastart];
                if ((*intor)(eribuf, shls, envs->atm, envs->natm,
                             envs->bas, envs->nbas, envs->env, cintopt)) {
                        for (i = 0; i < di; i++, i0++) {
                        for (j0 = ao_loc[jsh], j = 0; j < dj; j++, j0++) {
                                ij0 = i0 * nao + j0;
                                peri = eri + ij0 * naoaux + k0;
                                pbuf = eribuf + j * di + i;
                                for (k = 0; k < dk; k++) {
                                        peri[k] = pbuf[k*dij];
                                }
                        } }
                } else {
                        for (i = 0; i < di; i++, i0++) {
                        for (j0 = ao_loc[jsh], j = 0; j < dj; j++, j0++) {
                                ij0 = i0 * nao + j0;
                                peri = eri + ij0 * naoaux + k0;
                                for (k = 0; k < dk; k++) {
                                        peri[k] = 0;
                                }
                        } }
                }
        }
        free(eribuf);
}

void RIfill_r_s2ij_auxe2(int (*intor)(), double complex *eri,
                         int ish, int jsh, int bastart, int auxstart, int auxcount,
                         CINTOpt *cintopt, struct _VHFEnvs *envs)
{
        if (ish < jsh) {
                return;
        }

        const int *ao_loc = envs->ao_loc;
        const int di = ao_loc[ish+1] - ao_loc[ish];
        const int dj = ao_loc[jsh+1] - ao_loc[jsh];
        const int dij = di * dj;
        const int ijoff = ao_loc[bastart] * (ao_loc[bastart] + 1) / 2;
        const int nbasnaux = auxstart + auxcount;
        const int naoaux = ao_loc[nbasnaux] - ao_loc[auxstart];
        double complex *eribuf = malloc(sizeof(double complex)*di*dj*naoaux);

        int ksh, dk;
        int i, j, k, i0, j0, k0;
        int shls[3];
        size_t ij0;
        double complex *peri, *pbuf;

        shls[0] = ish;
        shls[1] = jsh;
        for (ksh = auxstart; ksh < nbasnaux; ksh++) {
                shls[2] = ksh;
                k0 = ao_loc[ksh] - ao_loc[auxstart];
                dk = ao_loc[ksh+1] - ao_loc[ksh];
                if ((*intor)(eribuf, shls, envs->atm, envs->natm,
                             envs->bas, envs->nbas, envs->env, cintopt)) {
                        if (ish == jsh) {
                                for (i0 = ao_loc[ish],i = 0; i < di; i++, i0++) {
                                for (j0 = ao_loc[jsh],j = 0; j0 <= i0; j++, j0++) {
                                        ij0 = i0*(i0+1)/2 + j0 - ijoff;
                                        peri = eri + ij0 * naoaux + k0;
                                        pbuf = eribuf + j * di + i;
                                        for (k = 0; k < dk; k++) {
                                                peri[k] = pbuf[k*dij];
                                        }
                                } }
                        } else {
                                for (i0 = ao_loc[ish], i = 0; i < di; i++,i0++) {
                                for (j0 = ao_loc[jsh], j = 0; j < dj; j++,j0++) {
                                        ij0 = i0*(i0+1)/2 + j0 - ijoff;
                                        peri = eri + ij0 * naoaux + k0;
                                        pbuf = eribuf + j * di + i;
                                        for (k = 0; k < dk; k++) {
                                                peri[k] = pbuf[k*dij];
                                        }
                                } }
                        }
                } else {
                        if (ish == jsh) {
                                for (i0 = ao_loc[ish],i = 0; i < di; i++, i0++) {
                                for (j0 = ao_loc[jsh],j = 0; j0 <= i0; j++, j0++) {
                                        ij0 = i0*(i0+1)/2 + j0 - ijoff;
                                        peri = eri + ij0 * naoaux + k0;
                                        for (k = 0; k < dk; k++) {
                                                peri[k] = 0;
                                        }
                                } }
                        } else {
                                for (i0 = ao_loc[ish], i = 0; i < di; i++,i0++) {
                                for (j0 = ao_loc[jsh], j = 0; j < dj; j++,j0++) {
                                        ij0 = i0*(i0+1)/2 + j0 - ijoff;
                                        peri = eri + ij0 * naoaux + k0;
                                        for (k = 0; k < dk; k++) {
                                                peri[k] = 0;
                                        }
                                } }
                        }
                }
        }
        free(eribuf);
}


/*
 * fill can be one of RIfill_s1_auxe2 and RIfill_s2ij_auxe2
 * NOTE nbas is the number of normal AO basis, the number of auxiliary
 * basis is given by auxcount;
 * bastart and bascount to fill a range of basis;
 * auxstart is the end of normal basis, so it equals to the number of
 * normal basis
 */
void RIr_3c2e_auxe2_drv(int (*intor)(), void (*fill)(), double complex *eri,
                        int bastart, int bascount, int auxstart, int auxcount,
                        int ncomp, CINTOpt *cintopt,
                        int *atm, int natm, int *bas, int nbas, double *env)
{
        const int nbasnaux = auxstart + auxcount;
        int i, j, ij;
        int *ao_loc = malloc(sizeof(int)*(nbasnaux+1));
// first nbas entries are spinor basis
        CINTshells_spinor_offset(ao_loc, bas, nbas);
        int n2c = ao_loc[nbas-1] + CINTcgto_spinor(nbas-1, bas);
        ao_loc[nbas] = n2c;
// the next auxcount functions can be real spheric
        ao_loc[auxstart] = n2c;
        for (i = auxstart; i < nbasnaux; i++) {
                ao_loc[i+1] = ao_loc[i] + CINTcgto_spheric(i, bas);
        }
        struct _VHFEnvs envs = {natm, nbas, atm, bas, env, n2c, ao_loc};


#pragma omp parallel default(none) \
        shared(eri, intor, fill, bastart, bascount, auxstart, auxcount, \
               nbas, envs, cintopt) \
        private(ij, i, j)
#pragma omp for nowait schedule(dynamic, 2)
        for (ij = bastart*nbas; ij < (bastart+bascount)*nbas; ij++) {
                i = ij / nbas;
                j = ij - i * nbas;
                (*fill)(intor, eri, i, j, bastart, auxstart, auxcount,
                        cintopt, &envs);
        }

        free(ao_loc);
}


/*
 * transform bra (without doing conj(mo)), v_{iq} = C_{pi} v_{pq}
 * s1 to label AO symmetry
 */
int RIhalfmmm_r_s1_bra_noconj(double complex *vout, double complex *vin,
                              struct _AO2MOEnvs *envs, int seekdim)
{
        switch (seekdim) {
                case 1: return envs->bra_count * envs->nao;
                case 2: return envs->nao * envs->nao;
        }
        const double complex Z0 = 0;
        const double complex Z1 = 1;
        const char TRANS_N = 'N';
        int n2c = envs->nao;
        int i_start = envs->bra_start;
        int i_count = envs->bra_count;
        double complex *mo_coeff = envs->mo_coeff;

        zgemm_(&TRANS_N, &TRANS_N, &n2c, &i_count, &n2c,
               &Z1, vin, &n2c, mo_coeff+i_start*n2c, &n2c,
               &Z0, vout, &n2c);
        return 0;
}

/*
 * transform ket, s1 to label AO symmetry
 */
int RIhalfmmm_r_s1_ket(double complex *vout, double complex *vin,
                       struct _AO2MOEnvs *envs, int seekdim)
{
        switch (seekdim) {
                case 1: return envs->nao * envs->ket_count;
                case 2: return envs->nao * envs->nao;
        }
        const double complex Z0 = 0;
        const double complex Z1 = 1;
        const char TRANS_T = 'T';
        const char TRANS_N = 'N';
        int n2c = envs->nao;
        int j_start = envs->ket_start;
        int j_count = envs->ket_count;
        double complex *mo_coeff = envs->mo_coeff;

        zgemm_(&TRANS_T, &TRANS_N, &j_count, &n2c, &n2c,
               &Z1, mo_coeff+j_start*n2c, &n2c, vin, &n2c,
               &Z0, vout, &j_count);
        return 0;
}

/*
 * transform bra (without doing conj(mo)), v_{iq} = C_{pi} v_{pq}
 * s2 to label AO symmetry
 */
int RIhalfmmm_r_s2_bra_noconj(double complex *vout, double complex *vin,
                              struct _AO2MOEnvs *envs, int seekdim)
{
        switch (seekdim) {
                case 1: return envs->bra_count * envs->nao;
                case 2: return envs->nao * (envs->nao+1) / 2;
        }
        const double complex Z0 = 0;
        const double complex Z1 = 1;
        const char SIDE_L = 'L';
        const char UPLO_U = 'U';
        int n2c = envs->nao;
        int i_start = envs->bra_start;
        int i_count = envs->bra_count;
        double complex *mo_coeff = envs->mo_coeff;

        zhemm_(&SIDE_L, &UPLO_U, &n2c, &i_count,
               &Z1, vin, &n2c, mo_coeff+i_start*n2c, &n2c,
               &Z0, vout, &n2c);
        return 0;
}

/*
 * transform ket, s2 to label AO symmetry
 */
int RIhalfmmm_r_s2_ket(double complex *vout, double complex *vin,
                       struct _AO2MOEnvs *envs, int seekdim)
{
        switch (seekdim) {
                case 1: return envs->nao * envs->ket_count;
                case 2: return envs->nao * (envs->nao+1) / 2;
        }
        const double complex Z0 = 0;
        const double complex Z1 = 1;
        const char SIDE_L = 'L';
        const char UPLO_U = 'U';
        int n2c = envs->nao;
        int j_start = envs->ket_start;
        int j_count = envs->ket_count;
        double complex *mo_coeff = envs->mo_coeff;
        double complex *buf = malloc(sizeof(double complex)*n2c*j_count);
        int i, j;

        zhemm_(&SIDE_L, &UPLO_U, &n2c, &j_count,
               &Z1, vin, &n2c, mo_coeff+j_start*n2c, &n2c,
               &Z0, buf, &n2c);
        for (j = 0; j < n2c; j++) {
                for (i = 0; i < j_count; i++) {
                        vout[i] = buf[i*n2c+j];
                }
                vout += j_count;
        }
        free(buf);
        return 0;
}

/*
 * unpack the AO integrals and copy to vout, s2 to label AO symmetry
 */
int RImmm_r_s2_copy(double complex *vout, double complex *vin,
                    struct _AO2MOEnvs *envs, int seekdim)
{
        switch (seekdim) {
                case 1: return envs->nao * envs->nao;
                case 2: return envs->nao * (envs->nao+1) / 2;
        }
        int n2c = envs->nao;
        int i, j;
        for (i = 0; i < n2c; i++) {
                for (j = 0; j < i; j++) {
                        vout[i*n2c+j] = vin[j];
                        vout[j*n2c+i] = conj(vin[j]);
                }
                vout[i*n2c+i] = vin[i];
                vin += n2c;
        }
        return 0;
}

/*
 * transpose (no conj) the AO integrals and copy to vout, s2 to label AO symmetry
 */
int RImmm_r_s2_transpose(double complex *vout, double complex *vin,
                         struct _AO2MOEnvs *envs, int seekdim)
{
        switch (seekdim) {
                case 1: return envs->nao * envs->nao;
                case 2: return envs->nao * (envs->nao+1) / 2;
        }
        int n2c = envs->nao;
        int i, j;
        for (i = 0; i < n2c; i++) {
                for (j = 0; j < i; j++) {
                        vout[j*n2c+i] = vin[j];
                        vout[i*n2c+j] = conj(vin[j]);
                }
                vout[i*n2c+i] = vin[i];
                vin += n2c;
        }
        return 0;
}


/*
 * ************************************************
 * s1, s2ij, s2kl, s4 here to label the AO symmetry
 */
void RItranse2_r_s1(int (*fmmm)(),
                    double complex *vout, double complex *vin, int row_id,
                    struct _AO2MOEnvs *envs)
{
        size_t ij_pair = (*fmmm)(NULL, NULL, envs, 1);
        size_t nao2 = envs->nao * envs->nao;
        (*fmmm)(vout+ij_pair*row_id, vin+nao2*row_id, envs, 0);
}

void RItranse2_r_s2(int (*fmmm)(),
                    double complex *vout, double complex *vin, int row_id,
                    struct _AO2MOEnvs *envs)
{
        int nao = envs->nao;
        size_t ij_pair = (*fmmm)(NULL, NULL, envs, 1);
        size_t nao2 = nao*(nao+1)/2;
        double complex *buf = malloc(sizeof(double complex) * nao*nao);
        NPzunpack_tril(nao, vin+nao2*row_id, buf, 0);
        (*fmmm)(vout+ij_pair*row_id, buf, envs, 0);
        free(buf);
}

