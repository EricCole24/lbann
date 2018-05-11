#!/bin/sh

DIRNAME=`dirname $0`
#Set Script Name variable
SCRIPT=`basename ${0}`

# Figure out which cluster we are on
CLUSTER=`hostname | sed 's/\([a-zA-Z][a-zA-Z]*\)[0-9]*/\1/g'`

RUN="srun"


TASKS_PER_NODE=12
export SLURM_NNODES=$SLURM_JOB_NUM_NODES

# Look for the binary in the cluster specific build directory
BINDIR="${DIRNAME}/../build/gnu.${CLUSTER}.llnl.gov${DEBUGDIR}/model_zoo"

#add whatever is on the command line to options
OPTS=""
for v in "$@"; do
  OPTS="$OPTS $v"
done

#over-ride optimizer, because learn_rate differs from that 
#specified in the prototext file
OPTS="$OPTS --opt=adam --learn_rate=.0001 --beta1=0.9 --beta2=0.999 --eps=1e-8"

TASKS=$((${SLURM_JOB_NUM_NODES} * ${SLURM_CPUS_ON_NODE}))
if [ ${TASKS} -gt 384 ]; then
TASKS=384
fi
LBANN_TASKS=$((${SLURM_NNODES} * ${TASKS_PER_NODE}))

CMD="${RUN} -n${LBANN_TASKS}  \
  --ntasks-per-node=${TASKS_PER_NODE} \
  ${BINDIR}/lbann \
  --model=../model_zoo/prototext/model_autoencoder_cifar10.prototext \
  --reader=../model_zoo/prototext/data_reader_cifar10.prototext \
  --optimizer=../model_zoo/prototext/opt_adam.prototext \
  $OPTS"

echo ${CMD}
${CMD}
